/*
 * Unlock state: spells, doors, the Marauder's Map, the shortcut and portrait passwords. Everything starts LOCKED
 * (poc8c: the ini no longer forces anything open; the gIni* values stay 0 and "(start)" never shows). Source:
 *   - the save: what has been FOUND (pickups, the debug panel; received items later). Kept in the game's PersistentStore
 *     as OC_<Section>_<Key> = "1", which travels between levels and is written into save games, so a New Game starts
 *     empty and loading a save brings its own state. Read on every settled map load (game thread).
 *
 * Spells: every spell check in the game goes through WeaponWand.HasSpell(class<Projectile>) (the spell cursor's
 * targeting, ChooseSpell; HarryWand / RonWand / HermioneWand only add their character exclusions and call super).
 * HasSpell is an UnrealScript function, and script-to-script calls never pass ProcessEvent, so it is turned into a
 * native one: UObject::CallFunction calls Function->Func whenever Function->iNative != 0 (Core.dll, retail:
 * `cmpw $0, 0x88(func)` then `call *0x94(func)` with ECX = the object, args (FFrame&, Result)). The function object
 * gets iNative = 1 and Func = hasspell_native, which reads its one argument from the caller's bytecode the way a
 * native does (FFrame::Step, then P_FINISH) and returns the vanilla answer (not None, not DraconisForsFireSpell)
 * unless the spell is locked. Before patching, Func must be UObject::ProcessInternal (what every script function
 * holds), which also confirms the two offsets. Lumos counts as a spell here: the wand light is lit by casting
 * LumosSpell at a gargoyle.
 *
 * Map: HPHeroController.GiveMap() (exec, the cheat the vanilla scene's GiveMap action mirrors), called on every map
 * load while [Items] MaraudersMap=1 (it does nothing if the map is already in the inventory).
 * Shortcuts: PortraitPasswordManager entries 8-21. On: EnablePortraitShortcuts(). Off: Owned = 0 for each (but 18,
 * which vanilla owns from the start).
 */
/* Native functions can't be called through ProcessEvent (an exec function reads its arguments from the script
   bytecode, not from the parms block), so SetCollision / SetCollisionSize / Destroy are called as the C++ methods
   Engine.dll exports. Shared with blocker.h. */
typedef void (__thiscall *setcol_fn)(void *actor, int colActors, int blockActors, int blockPlayers);
typedef void (__thiscall *setcolsize_fn)(void *actor, float radius, float height);
typedef int (__thiscall *destroyactor_fn)(void *level, void *actor, int bNetForce);
static setcol_fn gSetCollision; static setcolsize_fn gSetCollisionSize; static destroyactor_fn gDestroyActor;
static int offXLevel = -1;
static void engine_exports(void) {
    HANDLE eng; static wchar b[160];
    if (gSetCollision && gSetCollisionSize && gDestroyActor && offXLevel > 0) return;
    offXLevel = prop_offset(L"Actor", L"XLevel");
    eng = GetModuleHandleW(L"Engine.dll");
    if (eng) {
        gSetCollision = (setcol_fn)GetProcAddress(eng, "?SetCollision@AActor@@QAEXHHH@Z");
        gSetCollisionSize = (setcolsize_fn)GetProcAddress(eng, "?SetCollisionSize@AActor@@QAEXMM@Z");
        gDestroyActor = (destroyactor_fn)GetProcAddress(eng, "?DestroyActor@ULevel@@UAEHPAVAActor@@H@Z");
    }
    wsprintfW(b, L"engine: SetCollision %s, SetCollisionSize %s, DestroyActor %s; xlevel=%d",
              gSetCollision ? L"ok" : L"-", gSetCollisionSize ? L"ok" : L"-", gDestroyActor ? L"ok" : L"-", offXLevel);
    log_line(b);
}

struct spellent { const wchar *key, *cls; };
static const struct spellent SPELLS[] = {
    { L"Alohomora", L"AlohomoraSpell" },     { L"CarpeRetractum", L"RetractumSpell" }, { L"Depulso", L"DepulsoSpell" },
    { L"Draconifors", L"DraconisForsSpell" }, { L"ExpectoPatronum", L"PatronumSpell" }, { L"Glacius", L"GlaciusSpell" },
    { L"Lapifors", L"LapiforsSpell" },       { L"Lumos", L"LumosSpell" },             { L"Rictusempra", L"RictusempraSpell" },
    { L"Spongify", L"SpongifySpell" },
};
#define NSPELLS (int)(sizeof SPELLS / sizeof SPELLS[0])
static volatile int gSpellOn[NSPELLS] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };   /* -1 = not read yet (has it) */
static u32 gSpellCls[NSPELLS], gClsDracFire;
static volatile int gHaveMap = -1, gHaveShortcuts = -1;
static volatile int gSaveSpell[NSPELLS], gSaveMap, gSaveShortcuts;   /* found (save); gSaveDoor: opencastle.c */
static volatile int gIniSpell[NSPELLS], gIniMap, gIniShortcuts;       /* start state (ini), for the debug panel */
/* portrait passwords 1-8 = PortraitPasswordManager entries 0-7 (pp_0..pp_7, in the order the twins sell them) */
static volatile int gSavePortrait[8], gIniPortrait[8], gHavePortrait[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
typedef void (__thiscall *step_fn)(void *frame, void *context, void *result);
static step_fn gStep;

static int cls_child(u32 c, u32 want) { int guard = 64; while (c && guard--) { if (c == want) return 1; c = *(u32 *)(c + 0x2C); } return 0; }
/* WeaponWand.HasSpell, as a native (game thread) */
static void __thiscall hasspell_native(void *self, u32 *stack, u32 *result) {
    u32 cls = 0; int i, v; (void)self;
    gStep(stack, (void *)stack[2], &cls);                 /* P_GET_OBJECT(UClass, spellClass); FFrame.Object at +8 */
    stack[3]++;                                           /* P_FINISH: skip EX_EndFunctionParms (FFrame.Code at +0xC) */
    if (*(u8 *)stack[3] == 0x42) gStep(stack, (void *)stack[2], 0);   /* ... and debug info, as the engine's natives */
    v = cls && !(gClsDracFire && cls_child(cls, gClsDracFire));
    for (i = 0; v && i < NSPELLS; i++) if (!gSpellOn[i] && gSpellCls[i] && cls_child(cls, gSpellCls[i])) v = 0;
    if (result) *result = (u32)v;
}
struct fnfind { int name, outer, clsFunction; u32 found; };
static int v_fnfind(u32 o, int name, u32 cls, void *c) {
    struct fnfind *f = c; u32 outer;
    if (name != f->name || !cls || (int)rd32(cls + 0x20) != f->clsFunction) return 0;
    outer = rd32(o + 0x18);
    if (outer && (int)rd32(outer + 0x20) == f->outer) { f->found = o; return 1; }
    return 0;
}
/* worker, every 2 s until done */
static void spells_init(void) {
    static int state; static wchar b[300]; struct fnfind f; HANDLE core; u32 pi; int i, missing = 0;
    if (state) return;
    for (i = 0; i < NSPELLS; i++) if (!gSpellCls[i] && !(gSpellCls[i] = find_class(SPELLS[i].cls))) missing++;
    if (!gClsDracFire) gClsDracFire = find_class(L"DraconisForsFireSpell");
    f.name = name_index(L"HasSpell"); f.outer = name_index(L"WeaponWand"); f.clsFunction = name_index(L"Function"); f.found = 0;
    if (f.name < 0 || f.outer < 0 || f.clsFunction < 0 || missing || !gClsDracFire) return;   /* HGame not loaded yet */
    for_each_object(v_fnfind, &f);
    core = GetModuleHandleW(L"Core.dll");
    gStep = core ? (step_fn)GetProcAddress(core, "?Step@FFrame@@QAEXPAVUObject@@QAX@Z") : 0;
    pi = core ? (u32)GetProcAddress(core, "?ProcessInternal@UObject@@QAEXAAUFFrame@@QAX@Z") : 0;
    if (!f.found || !gStep || !pi) { state = -1; log_line(L"spells: WeaponWand.HasSpell / FFrame::Step not found; spells NOT gated"); return; }
    if ((rd32(f.found + 0x88) & 0xFFFF) != 0 || rd32(f.found + 0x94) != pi) {
        state = -1; wsprintfW(b, L"spells: HasSpell isn't a plain script function here (iNative %d, Func %08x vs %08x); spells NOT gated",
                              rd32(f.found + 0x88) & 0xFFFF, rd32(f.found + 0x94), pi); log_line(b); return;
    }
    *(volatile u32 *)(f.found + 0x94) = (u32)hasspell_native;
    *(volatile unsigned short *)(f.found + 0x88) = 1;
    state = 1;
    wsprintfW(b, L"spells: WeaponWand.HasSpell (%08x) now native; locked spells can't be targeted or cast", f.found); log_line(b);
}
/* worker, every 2 s (and right after a pickup): the ini switches */
static void unlocks_poll(void) {
    static wchar b[300]; int i, on, changed = 0;
    for (i = 0; i < NSPELLS; i++) {
        gIniSpell[i] = 0;   /* no ini start state any more (poc8c) */
        on = gIniSpell[i] || gSaveSpell[i];
        if (on != gSpellOn[i]) { gSpellOn[i] = on; changed = 1; }
    }
    if (changed) {
        wcpy(b, L"spells:", 300);
        for (i = 0; i < NSPELLS; i++) wsprintfW(b + wlen(b), L" %s=%d", SPELLS[i].key, gSpellOn[i]);
        log_line(b);
    }
    gIniMap = 0;
    on = gIniMap || gSaveMap;
    if (on != gHaveMap) { gHaveMap = on; wsprintfW(b, L"items: Marauder's Map %s", on ? L"unlocked" : L"locked"); log_line(b); if (on) gPendingShortcuts = 1; }
    gIniShortcuts = 0;
    on = gIniShortcuts || gSaveShortcuts;
    if (on != gHaveShortcuts) { gHaveShortcuts = on; wsprintfW(b, L"items: shortcut passwords %s", on ? L"unlocked" : L"locked"); log_line(b); gPendingShortcuts = 1; }
    for (i = 0; i < 8; i++) {
        wchar k[16]; wsprintfW(k, L"Portrait%d", i + 1);
        gIniPortrait[i] = 0; (void)k;
        on = gIniPortrait[i] || gSavePortrait[i];
        if (on != gHavePortrait[i]) { gHavePortrait[i] = on; wsprintfW(b, L"items: portrait password %d %s", i + 1, on ? L"unlocked" : L"locked"); log_line(b); gPendingShortcuts = 1; }
    }
}
/* worker, on the first map of a New Game: drop the previous game's found flags at once (the save read at settle
   would do it ~3 s later; until then the old game's spells would still work) */
static void unlocks_reset(void) {
    int i;
    for (i = 0; i < NSPELLS; i++) gSaveSpell[i] = 0;
    for (i = 0; i < 16; i++) gSaveDoor[i] = 0;
    for (i = 0; i < 8; i++) gSavePortrait[i] = 0;
    gSaveMap = gSaveShortcuts = 0;
    log_line(L"new game: every unlock reset to locked");
    unlocks_poll();
}
/* ---- the save side (game thread) ---- */
static u32 cur_store(void) {
    u32 level = cur_level(), game = level && offGame > 0 ? P(level + offGame) : 0;
    return game && offStore > 0 ? P(game + offStore) : 0;
}
static int save_flag_get(u32 store, const wchar *section, const wchar *key) {
    wchar k[96], v[16];
    wsprintfW(k, L"OC_%s_%s", section, key);
    return lo_restore_value(store, k, v, 16) && v[0] == '1';
}
/* every settled map load, before items_apply acts on it */
static void save_flags_read(void) {
    static wchar b[600]; u32 store = cur_store(); int i, n = 0;
    if (!store) { log_line(L"save flags: no PersistentStore on this map"); return; }
    wcpy(b, L"save flags (found):", 600);
    for (i = 0; i < NSPELLS; i++) if ((gSaveSpell[i] = save_flag_get(store, L"Spells", SPELLS[i].key)) != 0) { wsprintfW(b + wlen(b), L" %s", SPELLS[i].key); n++; }
    for (i = 0; i < NDOORS && i < 16; i++) if ((gSaveDoor[i] = save_flag_get(store, L"Doors", DOORS[i].key)) != 0) { wsprintfW(b + wlen(b), L" door:%s", DOORS[i].key); n++; }
    if ((gSaveMap = save_flag_get(store, L"Items", L"MaraudersMap")) != 0) { wcpy(b + wlen(b), L" MaraudersMap", 40); n++; }
    if ((gSaveShortcuts = save_flag_get(store, L"Items", L"ShortcutPasswords")) != 0) { wcpy(b + wlen(b), L" ShortcutPasswords", 40); n++; }
    for (i = 0; i < 8; i++) { wchar k[16]; wsprintfW(k, L"Portrait%d", i + 1);
        if ((gSavePortrait[i] = save_flag_get(store, L"Items", k)) != 0) { wsprintfW(b + wlen(b), L" %s", k); n++; } }
    if (!n) wcpy(b + wlen(b), L" none", 10);
    log_line(b);
    unlocks_poll();
}
/* a pickup (later: a received item) sets a flag; the debug panel can also clear one. Takes effect now. */
static void save_flag_put(const wchar *section, const wchar *key, int on) {
    static wchar k[96], b[160]; u32 store = cur_store();
    if (!store) { wsprintfW(b, L"save flags: no PersistentStore; %s %s not kept", section, key); log_line(b); return; }
    wsprintfW(k, L"OC_%s_%s", section, key);
    lo_save_value(store, k, on ? L"1" : L"0");
    wsprintfW(b, L"save flags: %s %s", k, on ? L"set" : L"cleared"); log_line(b);
    save_flags_read();
}
static void save_flag_set(const wchar *section, const wchar *key) { save_flag_put(section, key, 1); }
/* the carrier pawn holds the inventory (Level.InventoryCarrierPawn; GiveMap adds the map there) */
static int offCarrier = -1; static u32 gClsMapInv;
static u32 carrier_pawn(void) {
    u32 pc = gPlayer ? P(gPlayer + offPlayerActor) : 0, level = pc ? P(pc + offLevel) : 0;
    if (offCarrier < 0) offCarrier = prop_offset(L"LevelInfo", L"InventoryCarrierPawn");
    return level && offCarrier > 0 ? P(level + offCarrier) : 0;
}
/* game thread: Marauder's Map locked again (debug panel): carrier.FindInventoryType -> DeleteInventory -> destroy */
static void map_take_away(void) {
    struct { u32 cls, ret; } fp; struct { u32 item; } dp; void *fn; u32 carrier = carrier_pawn(), xl;
    static int N_FindInv = -1, N_DelInv = -1;
    if (!carrier) return;
    if (!gClsMapInv) gClsMapInv = find_class(L"MaraudersMap");
    if (N_FindInv < 0) N_FindInv = name_index(L"FindInventoryType");
    if (N_DelInv < 0) N_DelInv = name_index(L"DeleteInventory");
    if (!gClsMapInv || N_FindInv < 0 || !(fn = gFindFunction((void *)carrier, N_FindInv, 0))) return;
    fp.cls = gClsMapInv; fp.ret = 0; gPEOrig((void *)carrier, fn, &fp, 0);
    if (!fp.ret) return;
    if (N_DelInv >= 0 && (fn = gFindFunction((void *)carrier, N_DelInv, 0))) { dp.item = fp.ret; gPEOrig((void *)carrier, fn, &dp, 0); }
    xl = offXLevel > 0 ? P(fp.ret + offXLevel) : 0;
    if (xl && gDestroyActor) gDestroyActor((void *)xl, (void *)fp.ret, 0);
    log_line(L"items: Marauder's Map taken away (locked)");
}
/* game thread: portrait passwords 1-8 follow their flags (Owned of entries 0-7) */
static void portraits_apply(void) {
    u32 pc, level, game, ppm, pws; int i, n;
    if (!shop_init()) return;
    pc = P(gPlayer + offPlayerActor); level = pc ? P(pc + offLevel) : 0;
    game = level ? P(level + offGame) : 0; ppm = game ? P(game + offPPM) : 0;
    pws = ppm ? P(ppm + offPWs) : 0; n = ppm ? (int)P(ppm + offPWs + 4) : 0;
    if (!pws || n != 22) return;
    for (i = 0; i < 8; i++) *(u32 *)(pws + i * PW_STRIDE + offPwOwned) = gHavePortrait[i] > 0 ? 1 : 0;
}
/* game thread, on every settled map load and after a Map / Shortcut Passwords pickup */
static void items_apply(void) {
    static wchar b[200]; u32 pc, level, game, ppm, pws; int n, i, cleared = 0; void *fn;
    static int N_GiveMap = -1;
    if (!gPlayer) return;
    save_flags_read();
    if (gHaveShortcuts > 0) enable_shortcuts();
    else if (shop_init()) {
        pc = P(gPlayer + offPlayerActor); level = pc ? P(pc + offLevel) : 0;
        game = level ? P(level + offGame) : 0; ppm = game ? P(game + offPPM) : 0;
        pws = ppm ? P(ppm + offPWs) : 0; n = ppm ? (int)P(ppm + offPWs + 4) : 0;
        if (pws && n == 22) for (i = 8; i < 22; i++) if (i != 18 && P(pws + i * PW_STRIDE + offPwOwned)) { *(u32 *)(pws + i * PW_STRIDE + offPwOwned) = 0; cleared++; }
        wsprintfW(b, L"shortcuts: locked (%d password(s) cleared)", cleared); log_line(b);
    }
    if (gHaveMap > 0) {
        if (N_GiveMap < 0) N_GiveMap = name_index(L"GiveMap");
        pc = P(gPlayer + offPlayerActor);
        if (pc && N_GiveMap >= 0 && (fn = gFindFunction((void *)pc, N_GiveMap, 0))) { gPEOrig((void *)pc, fn, 0, 0); log_line(L"items: Marauder's Map given (GiveMap)"); }
        else log_line(L"items: GiveMap not found");
    } else map_take_away();
    portraits_apply();
}

/* game thread, tick job: switch the doors' own loading triggers (DOORS.trig) on or off. A ChangeLevelTrigger is a
   plain touch trigger: bCollideActors on, no blocking. */
static void trig_apply(void) {
    u32 bits = gTrigFire; int i, k;
    gTrigFire = 0;
    if (!gSetCollision) return;
    for (i = 0; i < MAXGATES; i++) if ((bits >> i) & 1)
        for (k = 0; k < 2; k++) if (gTrigObj[i][k]) gSetCollision((void *)gTrigObj[i][k], gTrigWant[i] ? 1 : 0, 0, 0);
}
