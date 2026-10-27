/*
 * Item look test: [Test] SpriteGrid=1 in HP3_OpenCastle.ini spawns every look the randomizer will use in a grid at
 * the player when Delete is pressed. (The earlier APLogos test, which redrew cakes / pasties / cards as AP logos, was
 * removed after it proved HP3AP.utx loads and renders.)
 * Needs from opencastle.c: gPEOrig-free (plain field writes + StaticLoadObject), log_line, offsets.
 */
typedef void *(__cdecl *loadobj_fn)(void *cls, void *outer, const wchar *name, const wchar *file, u32 flags, void *sandbox);
static loadobj_fn gLoadObject;
static u32 gClsTexture;
static int offDrawScale = -1, offTexture = -1, offStyle2 = -1;   /* Actor.DrawScale / Texture / Style (offDrawType: gates) */
static u32 gClsStaticMesh, gClsMesh, gClsClass; static int offStaticMesh = -1, offMesh = -1, N_SetCollision = -1;

/* ---- look test: pressing Delete in game spawns every item look the randomizer will use in a grid around the
   player (inert Engine.Effects actors for sprites and meshes). End stays MapTracker's mark key. */
/* the item looks the randomizer will use (decided 2026-10-26). kind: 'S' sprite texture, 'M' static mesh,
   'K' skeletal mesh, 'A' skeletal mesh with a looping idle, 'C' an actual class (spawned with its collision off, for looks that need its skins) */
struct grident { char kind; const wchar *obj; float scale; const wchar *label; };
static const struct grident GRID[] = {
    /* row 1: the ten spells */
    { 'S', L"SpellShapes.Shapes.SPSalohomora", 0, L"Alohomora" },        { 'S', L"SpellShapes.Shapes.SPSCarpeRetractum", 0, L"Carpe Retractum" },
    { 'S', L"SpellShapes.Shapes.SPSdepulso", 0, L"Depulso" },            { 'S', L"SpellShapes.Shapes.SPSdraconifor", 0, L"Draconifors" },
    { 'S', L"SpellShapes.Shapes.SPSexpecto", 0, L"Expecto Patronum" },   { 'S', L"SpellShapes.Shapes.SPSglacius", 0, L"Glacius" },
    { 'S', L"SpellShapes.Shapes.SPSlapifors", 0, L"Lapifors" },          { 'S', L"SpellShapes.Shapes.SPSlumos", 0, L"Lumos" },
    { 'S', L"SpellShapes.Shapes.SPSrictusempra", 0, L"Rictusempra" },    { 'S', L"SpellShapes.Shapes.SPSspongify", 0, L"Spongify" },
    /* row 2: Archipelago tiers and the level-unlock key */
    { 'S', L"HP3AP.APLogoProgression", 0, L"AP progression" }, { 'S', L"HP3AP.APLogoUseful", 0, L"AP useful" },
    { 'S', L"HP3AP.APLogoFiller", 0, L"AP filler" },           { 'S', L"HP3AP.APKeyDrawn", 0, L"level unlock key" },
    /* row 3: HP3's own models */
    { 'K', L"HProps.MarauderMap", 1.0f, L"Marauder's Map" },
    { 'K', L"HProps.PeevesScroll", 1.0f, L"password scroll (Peeves)" },
    { 'M', L"HP3_InteriorAccents_SM.books.BlueBandScroll_SM", 1.0f, L"password scroll (shop)" },
    { 'M', L"HP3_GamePlay_sm.Spells_Objects.PumpkinPasty_SM", 2.0f, L"pumpkin pasty" },
    { 'M', L"HP3_GamePlay_sm.Spells_Objects.CauldronCake_SM", 2.5f, L"cauldron cake" },
    { 'C', L"HGame.WC_HarryPotter", 0, L"wizard card (Harry Potter)" },
    /* row 4: challenge unlocks (decided 2026-10-27), animated skeletal meshes. Scales from the meshes' bounding
       spheres (HPCharacters.ukx): Buckbeak r 330, Pixie 78, MBOM 60 -> all about r 50 here. */
    { 'A', L"HPCharacters.Buckbeak", 0.15f, L"Hippogriff Challenge Unlock" },
    { 'A', L"HPCharacters.Pixie", 0.65f, L"Doxy Challenge Unlock" },
    { 'A', L"HPCharacters.MBOM", 0.8f, L"Monster Book Challenge Unlock" },
};
static const int GRIDROW[] = { 10, 4, 6, 3 };   /* entries per row, in order */
/* idle animation to loop, per 'A' look (first one the mesh has). Every sequence name is logged once, to pick from. */
static const wchar *const ANIM_TRY[][4] = {
    { L"standIdle", L"Stand_idle", L"Idle", L"Idle1" },   /* Buckbeak */
    { L"Hover_idle", L"Idle", 0, 0 },                     /* Pixie (Poxie.IdleAnimName) */
    { L"Idle", 0, 0, 0 },                                 /* MBOM (MonsterBook.StateIdle loops 'Idle') */
};
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
        if ((nm = name_index(ANIM_TRY[which][k])) >= 0 && gAnimNamed(inst, nm)) {
            gPlayAnim((void *)actor, 0, nm, 1.0f, 0.0f, 1);
            wsprintfW(b, L"grid: %s loops %s", label, ANIM_TRY[which][k]); log_line(b); return;
        }
    wsprintfW(b, L"grid: %s: none of the idle names found; reference pose", label); log_line(b);
}
#define NGRID (int)(sizeof GRID / sizeof GRID[0])
static float gGridAt[3];
static wchar gMapLogPath[600];
static int N_FancySpawn2 = -1; static u32 gClsEffects;
static int offUSize = -1;

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
/* worker, every poll on a settled map: Delete pressed -> grid around the player */
static void grid_poll(u32 pawn) {
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
    if (!gClsTexture) gClsTexture = find_class(L"Texture");
    if (!gClsStaticMesh) gClsStaticMesh = find_class(L"StaticMesh");
    if (!gClsMesh) gClsMesh = find_class(L"Mesh");
    if (!gClsClass) gClsClass = find_class(L"Class");
    if (offStaticMesh < 0) offStaticMesh = prop_offset(L"Actor", L"StaticMesh");
    if (offMesh < 0) offMesh = prop_offset(L"Actor", L"Mesh");
    if (N_SetCollision < 0) N_SetCollision = name_index(L"SetCollision");
    if (offDrawScale <= 0) { offDrawScale = prop_offset(L"Actor", L"DrawScale"); offTexture = prop_offset(L"Actor", L"Texture"); offStyle2 = prop_offset(L"Actor", L"Style"); }
    gGridPending = 1;
}

/* game thread */
static void grid_spawn(void) {
    struct { u32 type, owner; int tag; float x, y, z; int p, yw, r; u32 bz; u32 ret; } parms;
    static wchar b[240]; u32 pc, pawn, obj, cls; void *fn; int i, ok = 0, row = 0, col = 0, inrow = GRIDROW[0], nanim = 0;
    pc = gPlayer ? P(gPlayer + offPlayerActor) : 0; pawn = pc ? P(pc + offPawn) : 0;
    if (!pawn || !gClsEffects || !gLoadObject || !gClsTexture || offDrawScale <= 0 || offTexture <= 0 || offStyle2 <= 0
        || N_FancySpawn2 < 0 || !(fn = gFindFunction((void *)pawn, N_FancySpawn2, 0))) { log_line(L"grid: lookups failed"); return; }
    for (i = 0; i < NGRID; i++, col++) {
        const struct grident *g = &GRID[i]; float px;
        if (col == inrow) { row++; col = 0; inrow = GRIDROW[row]; }
        cls = g->kind == 'S' ? gClsTexture : g->kind == 'M' ? gClsStaticMesh : (g->kind == 'K' || g->kind == 'A') ? gClsMesh : gClsClass;
        obj = cls ? (u32)gLoadObject((void *)cls, 0, g->obj, 0, 0x2002, 0) : 0;
        if (!obj) { wsprintfW(b, L"grid: %s (%s) not found", g->label, g->obj); log_line(b); continue; }
        memset(&parms, 0, sizeof parms);
        parms.type = g->kind == 'C' ? obj : gClsEffects;
        parms.x = gGridAt[0] + (col - (inrow - 1) / 2.0f) * (row == 3 ? 120.0f : 70.0f); parms.y = gGridAt[1] + (row - 1) * 80.0f + (row == 3 ? 40.0f : 0.0f);
        parms.z = gGridAt[2] + 40.0f;
        gPEOrig((void *)pawn, fn, &parms, 0);
        if (!parms.ret) { wsprintfW(b, L"grid: spawn for %s failed", g->label); log_line(b); continue; }
        if (g->kind == 'S') {
            px = offUSize > 0 ? (float)(int)P(obj + offUSize) : 64.0f; if (px <= 0) px = 64.0f;
            *(u32 *)(parms.ret + offTexture) = obj; *(u8 *)(parms.ret + offStyle2) = 2;
            *(float *)(parms.ret + offDrawScale) = 38.0f / px;
        } else if (g->kind == 'M' && offStaticMesh > 0) {
            *(u32 *)(parms.ret + offStaticMesh) = obj; *(u8 *)(parms.ret + offDrawType) = 8; *(float *)(parms.ret + offDrawScale) = g->scale;
        } else if (g->kind == 'K' && offMesh > 0) {
            *(u32 *)(parms.ret + offMesh) = obj; *(u8 *)(parms.ret + offDrawType) = 2; *(float *)(parms.ret + offDrawScale) = g->scale;
        } else if (g->kind == 'A' && offMesh > 0) {
            *(u32 *)(parms.ret + offMesh) = obj; *(u8 *)(parms.ret + offDrawType) = 2; *(float *)(parms.ret + offDrawScale) = g->scale;
            grid_animate(parms.ret, obj, nanim, g->label); nanim++;
        } else if (g->kind == 'C') {   /* a real pickup: switch its collision off so it can't be collected */
            void *sc = N_SetCollision >= 0 ? gFindFunction((void *)parms.ret, N_SetCollision, 0) : 0;
            struct { u32 a, b, c; } cp = { 0, 0, 0 };
            if (sc) gPEOrig((void *)parms.ret, sc, &cp, 0); else log_line(L"grid: SetCollision not found; the card can be collected");
        }
        ok++;
    }
    wsprintfW(b, L"grid: %d of %d item looks placed around (%d, %d, %d)", ok, NGRID, (int)gGridAt[0], (int)gGridAt[1], (int)gGridAt[2]);
    log_line(b);
}
