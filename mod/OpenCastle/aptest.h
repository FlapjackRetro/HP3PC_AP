/*
 * Item test grid: pressing Delete (game window focused) spawns one of every item look the randomizer will use, in a
 * grid starting 150 units in front of (north of) the player. Each one falls to the floor and is picked up by walking
 * into it, and does what the real item will do:
 *   spell glyphs        [Spells] <spell>=1        (the spell becomes castable)
 *   drawn key           [Doors] Paddock=1         (test: the entrance-hall north door)
 *   Marauder's Map      [Items] MaraudersMap=1    (HPHeroController.GiveMap)
 *   password scroll     [Items] ShortcutPasswords=1 (EnablePortraitShortcuts)
 *   hippogriff / doxy / Monster Book   [Doors] Hippogriff / Doxy / MonsterBook = 1
 *   Archipelago logos   nothing (they still fall and can be picked up)
 *   pasty, cake, Harry Potter card     the game's own pickup classes, collected the vanilla way
 * Our looks are inert Engine.Effects actors: a sprite / static mesh / skeletal mesh, a collision cylinder
 * (SetCollisionSize + SetCollision through Engine.dll), bCollideWorld and PHYS_Falling. Pickup is a distance check on
 * the worker (the same reach as a vanilla pickup), then the game thread destroys the actor.
 * End stays MapTracker's mark key.
 */
typedef void *(__cdecl *loadobj_fn)(void *cls, void *outer, const wchar *name, const wchar *file, u32 flags, void *sandbox);
static loadobj_fn gLoadObject;
static u32 gClsTexture;
static int offDrawScale = -1, offTexture = -1, offStyle2 = -1;   /* Actor.DrawScale / Texture / Style (offDrawType: gates) */
static u32 gClsStaticMesh, gClsMesh, gClsClass; static int offStaticMesh = -1, offMesh = -1, N_SetCollision = -1;
static int offPhysics = -1, offCollideWorld = -1; static u32 maskCollideWorld;

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

/* kind: 'S' sprite texture, 'M' static mesh, 'K' skeletal mesh, 'A' skeletal mesh with a looping idle, 'C' one of the
   game's own pickup classes (spawned as is). fx: 's' spell, 'd' door, 'm' Marauder's Map, 'p' shortcut passwords,
   0 nothing. h: collision half-height, so the look rests on the floor (sprites are 38 units tall; the creatures
   from their meshes' lowest point x scale). */
struct grident { char kind; const wchar *obj; float scale; const wchar *label; char fx; const wchar *arg; float h; };
static const struct grident GRID[] = {
    /* row 1: the ten spells */
    { 'S', L"SpellShapes.Shapes.SPSalohomora", 0, L"Alohomora", 's', L"Alohomora", 19 },
    { 'S', L"SpellShapes.Shapes.SPSCarpeRetractum", 0, L"Carpe Retractum", 's', L"CarpeRetractum", 19 },
    { 'S', L"SpellShapes.Shapes.SPSdepulso", 0, L"Depulso", 's', L"Depulso", 19 },
    { 'S', L"SpellShapes.Shapes.SPSdraconifor", 0, L"Draconifors", 's', L"Draconifors", 19 },
    { 'S', L"SpellShapes.Shapes.SPSexpecto", 0, L"Expecto Patronum", 's', L"ExpectoPatronum", 19 },
    { 'S', L"SpellShapes.Shapes.SPSglacius", 0, L"Glacius", 's', L"Glacius", 19 },
    { 'S', L"SpellShapes.Shapes.SPSlapifors", 0, L"Lapifors", 's', L"Lapifors", 19 },
    { 'S', L"SpellShapes.Shapes.SPSlumos", 0, L"Lumos", 's', L"Lumos", 19 },
    { 'S', L"SpellShapes.Shapes.SPSrictusempra", 0, L"Rictusempra", 's', L"Rictusempra", 19 },
    { 'S', L"SpellShapes.Shapes.SPSspongify", 0, L"Spongify", 's', L"Spongify", 19 },
    /* row 2: Archipelago tiers (inert) and the level-unlock key (test: Paddock) */
    { 'S', L"HP3AP.APLogoProgression", 0, L"AP progression", 0, 0, 19 },
    { 'S', L"HP3AP.APLogoUseful", 0, L"AP useful", 0, 0, 19 },
    { 'S', L"HP3AP.APLogoFiller", 0, L"AP filler", 0, 0, 19 },
    { 'S', L"HP3AP.APKeyDrawn", 0, L"Paddock Unlock (key)", 'd', L"Paddock", 19 },
    /* row 3: HP3's own models */
    { 'K', L"HProps.MarauderMap", 1.0f, L"Marauder's Map", 'm', 0, 8 },
    { 'M', L"HP3_InteriorAccents_SM.books.BlueBandScroll_SM", 1.0f, L"Shortcut Passwords (scroll)", 'p', 0, 8 },
    { 'C', L"HGame.PumpkinPasty", 0, L"pumpkin pasty", 0, 0, 0 },
    { 'C', L"HGame.CauldronCake", 0, L"cauldron cake", 0, 0, 0 },
    { 'C', L"HGame.WC_HarryPotter", 0, L"wizard card (Harry Potter)", 0, 0, 0 },
    /* row 4: challenge unlocks (decided 2026-10-27), animated. Scales from the meshes' bounding spheres
       (HPCharacters.ukx): Buckbeak r 330, Pixie 78, MBOM 60; Buckbeak at 0.30 after the first look. */
    { 'A', L"HPCharacters.Buckbeak", 0.30f, L"Hippogriff Challenge Unlock", 'd', L"Hippogriff", 46 },
    { 'A', L"HPCharacters.Pixie", 0.65f, L"Doxy Challenge Unlock", 'd', L"Doxy", 30 },
    { 'A', L"HPCharacters.MBOM", 0.8f, L"Monster Book Challenge Unlock", 'd', L"MonsterBook", 28 },
};
static const int GRIDROW[] = { 10, 4, 5, 3 };   /* entries per row, in order */
#define NGRID (int)(sizeof GRID / sizeof GRID[0])
#define GRID_REACH 22.0f   /* horizontal pickup reach beyond the look's own radius (about the hero's radius) */
#define GRID_R     20.0f   /* collision radius of every look */
/* idle animation to loop, per 'A' look (first one the mesh has; the names are logged). */
static const wchar *const ANIM_TRY[][4] = {
    { L"Idle", 0, 0, 0 },                 /* Buckbeak: Fidget_Preen Flap Idle run RunToTakeoff Nip */
    { L"Hover_idle", L"Stand_idle", 0, 0 },   /* Pixie (Poxie.IdleAnimName) */
    { L"Idle", 0, 0, 0 },                 /* MBOM (MonsterBook.StateIdle loops 'Idle') */
};
static float gGridAt[3];
static wchar gMapLogPath[600];
static int N_FancySpawn2 = -1; static u32 gClsEffects;
static int offUSize = -1;
static u32 gGridObj[NGRID], gGridLevel;            /* live looks (0 = none / taken) */
static volatile u32 gGridKill[NGRID];               /* taken: the game thread destroys it */

/* Engine.dll: mesh instances and AActor::PlayAnim (native, so not through ProcessEvent) */
typedef void *(__thiscall *meshinst_fn)(void *mesh, void *actor);
typedef int   (__thiscall *animcount_fn)(void *inst);
typedef void *(__thiscall *animidx_fn)(void *inst, int i);
typedef int  *(__thiscall *animname_fn)(void *inst, int *ret, void *seq);
typedef void *(__thiscall *animnamed_fn)(void *inst, int name);
typedef void  (__thiscall *playanim_fn)(void *actor, int channel, int seq, float rate, float tween, int loop);
static meshinst_fn gMeshInst; static animcount_fn gAnimCount; static animidx_fn gAnimIdx; static animname_fn gAnimName;
static animnamed_fn gAnimNamed; static playanim_fn gPlayAnim;
/* game thread: loop an idle on a freshly spawned skeletal look; logs the mesh's sequences the first time */
static void grid_animate(u32 actor, u32 mesh, int which, const wchar *label) {
    static wchar b[1200]; static int logged[8]; void *inst; int n, i, k, nm;
    if (!gMeshInst || !gAnimCount || !gAnimIdx || !gAnimName || !gAnimNamed || !gPlayAnim) { log_line(L"grid: animation exports missing"); return; }
    if (!(inst = gMeshInst((void *)mesh, (void *)actor))) { wsprintfW(b, L"grid: %s has no mesh instance", label); log_line(b); return; }
    n = gAnimCount(inst);
    if (which < 8 && !logged[which]) {
        wchar one[64]; int len;
        logged[which] = 1; wsprintfW(b, L"grid: %s, %d animations:", label, n); len = wlen(b);
        for (i = 0; i < n && len < 1100; i++) {
            int out = -1; gAnimName(inst, &out, gAnimIdx(inst, i));
            if (out >= 0 && name_of(out, one, 64)) { wsprintfW(b + len, L" %s", one); len = wlen(b); }
        }
        log_line(b);
    }
    for (k = 0; k < 4 && which < 3 && ANIM_TRY[which][k]; k++)
        if ((nm = name_index(ANIM_TRY[which][k])) >= 0 && gAnimNamed(inst, nm)) { gPlayAnim((void *)actor, 0, nm, 1.0f, 0.0f, 1); return; }
    wsprintfW(b, L"grid: %s: none of the idle names found; reference pose", label); log_line(b);
}

/* key thread: the Delete key (only while the game window has focus) asks for a grid */
static volatile int gGridKey;
static u32 WINAPI key_thread(void *arg) {
    int was = 0; u32 pid = 0; (void)arg;
    for (;;) {
        int down;
        Sleep(30);
        down = (GetAsyncKeyState(0x2E /*VK_DELETE*/) & 0x8000) != 0;
        if (down && !was) {
            HWND w = GetForegroundWindow();
            GetWindowThreadProcessId(w, &pid);
            if (pid == GetCurrentProcessId()) gGridKey = 1;
        }
        was = down;
    }
}
/* worker: a look was walked into */
static void grid_pickup(int i) {
    const struct grident *g = &GRID[i]; static wchar b[200];
    wsprintfW(b, L"grid: picked up %s", g->label); log_line(b);
    switch (g->fx) {
    case 's': unlock_write(L"Spells", g->arg); break;
    case 'd': unlock_write(L"Doors", g->arg); break;   /* gates_poll opens it within 2 s */
    case 'm': unlock_write(L"Items", L"MaraudersMap"); break;
    case 'p': unlock_write(L"Items", L"ShortcutPasswords"); break;
    default: break;
    }
}
/* worker, every poll on a settled map: Delete pressed -> grid; and the pickup check */
static void grid_poll(u32 pawn, u32 level) {
    int i;
    if (level != gGridLevel) { for (i = 0; i < NGRID; i++) { gGridObj[i] = 0; gGridKill[i] = 0; } gGridLevel = level; }
    if (pawn && !gGridTake) {
        float me[3], at[3];
        if (rd(pawn + offLoc, me, 12))
            for (i = 0; i < NGRID; i++) {
                float dx, dy, dz, reach = GRID_R + GRID_REACH;
                if (!gGridObj[i] || !rd(gGridObj[i] + offLoc, at, 12)) continue;
                dx = at[0] - me[0]; dy = at[1] - me[1]; dz = at[2] - me[2];
                if (dx * dx + dy * dy > reach * reach || dz > GRID[i].h + 60 || dz < -(GRID[i].h + 60)) continue;
                gGridKill[i] = gGridObj[i]; gGridObj[i] = 0; gGridTake |= 1u << i;
                grid_pickup(i);
            }
    }
    if (!gGridKey || gGridPending) return;
    gGridKey = 0;
    if (!pawn || !rd(pawn + offLoc, gGridAt, 12)) return;
    if (!gClsEffects) gClsEffects = find_class(L"Effects");
    if (offUSize < 0) offUSize = prop_offset(L"BitmapMaterial", L"USize");
    if (N_FancySpawn2 < 0) N_FancySpawn2 = name_index(L"FancySpawn");
    if (!gLoadObject) { HANDLE core = GetModuleHandleW(L"Core.dll");
        gLoadObject = core ? (loadobj_fn)GetProcAddress(core, "?StaticLoadObject@UObject@@SAPAV1@PAVUClass@@PAV1@PBG2KPAVUPackageMap@@@Z") : 0; }
    if (!gPlayAnim) { HANDLE eng = GetModuleHandleW(L"Engine.dll");
        if (eng) {
            gMeshInst  = (meshinst_fn)GetProcAddress(eng, "?MeshGetInstance@UMesh@@UAEPAVUMeshInstance@@PBVAActor@@@Z");
            gAnimCount = (animcount_fn)GetProcAddress(eng, "?GetAnimCount@USkeletalMeshInstance@@UAEHXZ");
            gAnimIdx   = (animidx_fn)GetProcAddress(eng, "?GetAnimIndexed@USkeletalMeshInstance@@UAEPAXH@Z");
            gAnimName  = (animname_fn)GetProcAddress(eng, "?AnimGetName@USkeletalMeshInstance@@UAE?AVFName@@PAX@Z");
            gAnimNamed = (animnamed_fn)GetProcAddress(eng, "?GetAnimNamed@USkeletalMeshInstance@@UAEPAXVFName@@@Z");
            gPlayAnim  = (playanim_fn)GetProcAddress(eng, "?PlayAnim@AActor@@UAEXHVFName@@MMH@Z");
        } }
    engine_exports();
    if (!gClsTexture) gClsTexture = find_class(L"Texture");
    if (!gClsStaticMesh) gClsStaticMesh = find_class(L"StaticMesh");
    if (!gClsMesh) gClsMesh = find_class(L"Mesh");
    if (!gClsClass) gClsClass = find_class(L"Class");
    if (offStaticMesh < 0) offStaticMesh = prop_offset(L"Actor", L"StaticMesh");
    if (offMesh < 0) offMesh = prop_offset(L"Actor", L"Mesh");
    if (offPhysics < 0) offPhysics = prop_offset(L"Actor", L"Physics");
    if (offCollideWorld < 0) {
        u32 bp = prop_object(L"Actor", L"bCollideWorld"), m = bp ? rd32(bp + 0x64) : 0;
        if (bp && m && !(m & (m - 1))) { offCollideWorld = (int)rd32(bp + 0x4C); maskCollideWorld = m; }
    }
    if (offDrawScale <= 0) { offDrawScale = prop_offset(L"Actor", L"DrawScale"); offTexture = prop_offset(L"Actor", L"Texture"); offStyle2 = prop_offset(L"Actor", L"Style"); }
    gGridPending = 1;
}

/* game thread: destroy the looks the worker saw picked up; the Map / Shortcut Passwords take effect now */
static void grid_take(void) {
    u32 bits = gGridTake; int i, apply = 0;
    for (i = 0; i < NGRID; i++) if ((bits >> i) & 1) {
        u32 a = gGridKill[i], xl = a && offXLevel > 0 ? P(a + offXLevel) : 0;
        if (xl && gDestroyActor) gDestroyActor((void *)xl, (void *)a, 0);
        gGridKill[i] = 0;
        if (GRID[i].fx == 'm' || GRID[i].fx == 'p') apply = 1;
    }
    gGridTake = 0;
    if (apply) items_apply();
}
/* game thread */
static void grid_spawn(void) {
    struct { u32 type, owner; int tag; float x, y, z; int p, yw, r; u32 bz; u32 ret; } parms;
    static wchar b[240]; u32 pc, pawn, obj, cls, a; void *fn; int i, ok = 0, row = 0, col = 0, inrow = GRIDROW[0], nanim = 0;
    pc = gPlayer ? P(gPlayer + offPlayerActor) : 0; pawn = pc ? P(pc + offPawn) : 0;
    if (!pawn || !gClsEffects || !gLoadObject || !gClsTexture || offDrawScale <= 0 || offTexture <= 0 || offStyle2 <= 0
        || N_FancySpawn2 < 0 || !(fn = gFindFunction((void *)pawn, N_FancySpawn2, 0))) { log_line(L"grid: lookups failed"); return; }
    if (!gSetCollision || !gSetCollisionSize || offPhysics <= 0 || offCollideWorld <= 0)
        log_line(L"grid: collision / physics lookups missing; the looks will float and can't be picked up");
    for (i = 0; i < NGRID; i++, col++) {
        const struct grident *g = &GRID[i]; float px;
        if (col == inrow) { row++; col = 0; inrow = GRIDROW[row]; }
        if (gGridObj[i]) { u32 xl = offXLevel > 0 ? P(gGridObj[i] + offXLevel) : 0;   /* a second Delete replaces the grid */
            if (xl && gDestroyActor) gDestroyActor((void *)xl, (void *)gGridObj[i], 0);
            gGridObj[i] = 0; }
        cls = g->kind == 'S' ? gClsTexture : g->kind == 'M' ? gClsStaticMesh : (g->kind == 'K' || g->kind == 'A') ? gClsMesh : gClsClass;
        obj = cls ? (u32)gLoadObject((void *)cls, 0, g->obj, 0, 0x2002, 0) : 0;
        if (!obj) { wsprintfW(b, L"grid: %s (%s) not found", g->label, g->obj); log_line(b); continue; }
        memset(&parms, 0, sizeof parms);
        parms.type = g->kind == 'C' ? obj : gClsEffects;
        parms.x = gGridAt[0] + (col - (inrow - 1) / 2.0f) * (row == 3 ? 120.0f : 70.0f);
        parms.y = gGridAt[1] + 150.0f + row * 80.0f + (row == 3 ? 40.0f : 0.0f);
        parms.z = gGridAt[2] + 40.0f;
        gPEOrig((void *)pawn, fn, &parms, 0);
        if (!(a = parms.ret)) { wsprintfW(b, L"grid: spawn for %s failed", g->label); log_line(b); continue; }
        ok++;
        if (g->kind == 'C') continue;   /* the game's own pickup: it falls and is collected by itself */
        if (g->kind == 'S') {
            px = offUSize > 0 ? (float)(int)P(obj + offUSize) : 64.0f; if (px <= 0) px = 64.0f;
            *(u32 *)(a + offTexture) = obj; *(u8 *)(a + offStyle2) = 2;
            *(float *)(a + offDrawScale) = 38.0f / px;
        } else if (g->kind == 'M' && offStaticMesh > 0) {
            *(u32 *)(a + offStaticMesh) = obj; *(u8 *)(a + offDrawType) = 8; *(float *)(a + offDrawScale) = g->scale;
        } else if ((g->kind == 'K' || g->kind == 'A') && offMesh > 0) {
            *(u32 *)(a + offMesh) = obj; *(u8 *)(a + offDrawType) = 2; *(float *)(a + offDrawScale) = g->scale;
            if (g->kind == 'A') { grid_animate(a, obj, nanim, g->label); nanim++; }
        }
        /* hitbox, world collision, gravity */
        if (gSetCollisionSize) gSetCollisionSize((void *)a, GRID_R, g->h);
        if (gSetCollision) gSetCollision((void *)a, 1, 0, 0);
        if (offCollideWorld > 0) *(u32 *)(a + offCollideWorld) |= maskCollideWorld;
        if (offPhysics > 0) *(u8 *)(a + offPhysics) = 2;   /* PHYS_Falling */
        gGridObj[i] = a;
    }
    wsprintfW(b, L"grid: %d of %d items placed in front of (%d, %d, %d)", ok, NGRID, (int)gGridAt[0], (int)gGridAt[1], (int)gGridAt[2]);
    log_line(b);
}
