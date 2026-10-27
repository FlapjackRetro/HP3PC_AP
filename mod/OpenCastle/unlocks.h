/*
 * Unlock state: spells, the Marauder's Map and the shortcut passwords (doors live in [Doors], see gates_poll).
 * Read from HP3_OpenCastle.ini every 2 s, like the doors; the randomizer client will drive the same switches.
 *   [Spells] Alohomora / CarpeRetractum / Depulso / Draconifors / ExpectoPatronum / Glacius / Lapifors / Lumos /
 *            Rictusempra / Spongify = 1 (has it, default) or 0 (locked)
 *   [Items]  MaraudersMap = 0 (default) / 1, ShortcutPasswords = 0 (default) / 1
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
        on = GetPrivateProfileIntW(L"Spells", SPELLS[i].key, 1, gIniPath) != 0;
        if (on != gSpellOn[i]) { gSpellOn[i] = on; changed = 1; }
    }
    if (changed) {
        wcpy(b, L"spells:", 300);
        for (i = 0; i < NSPELLS; i++) wsprintfW(b + wlen(b), L" %s=%d", SPELLS[i].key, gSpellOn[i]);
        log_line(b);
    }
    on = GetPrivateProfileIntW(L"Items", L"MaraudersMap", 0, gIniPath) != 0;
    if (on != gHaveMap) { gHaveMap = on; wsprintfW(b, L"items: Marauder's Map %s", on ? L"unlocked" : L"locked"); log_line(b); if (on) gPendingShortcuts = 1; }
    on = GetPrivateProfileIntW(L"Items", L"ShortcutPasswords", 0, gIniPath) != 0;
    if (on != gHaveShortcuts) { gHaveShortcuts = on; wsprintfW(b, L"items: shortcut passwords %s", on ? L"unlocked" : L"locked"); log_line(b); gPendingShortcuts = 1; }
}
static void unlock_write(const wchar *section, const wchar *key) {
    WritePrivateProfileStringW(section, key, L"1", gIniPath);
    unlocks_poll();
}
/* game thread, on every settled map load and after a Map / Shortcut Passwords pickup */
static void items_apply(void) {
    static wchar b[200]; u32 pc, level, game, ppm, pws; int n, i, cleared = 0; void *fn;
    static int N_GiveMap = -1;
    if (!gPlayer) return;
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
    }
}
