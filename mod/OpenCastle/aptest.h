/*
 * Item look test: [Test] SpriteGrid=1 in HP3_OpenCastle.ini spawns every look the randomizer will use in a grid at
 * the player's latest End-key mark. (The earlier APLogos test, which redrew cakes / pasties / cards as AP logos, was
 * removed after it proved HP3AP.utx loads and renders.)
 * Needs from opencastle.c: gPEOrig-free (plain field writes + StaticLoadObject), log_line, offsets.
 */
typedef void *(__cdecl *loadobj_fn)(void *cls, void *outer, const wchar *name, const wchar *file, u32 flags, void *sandbox);
static loadobj_fn gLoadObject;
static u32 gClsTexture;
static int offDrawScale = -1, offTexture = -1, offStyle2 = -1;   /* Actor.DrawScale / Texture / Style (offDrawType: gates) */
static u32 gClsStaticMesh, gClsMesh, gClsClass; static int offStaticMesh = -1, offMesh = -1, N_SetCollision = -1;

/* ---- one-time test: every candidate item sprite in a grid where the player last pressed the MapTracker mark key
   (End). With [Test] SpriteGrid=1 the worker watches HP3_MapLog.txt; each new MARK on the current map spawns the grid
   (inert Engine.Effects actors: no collision, sprite drawn with the texture), 7 columns, 60 units apart:
     row 1: AP progression / useful / filler, keys: retail Key2, Key2 upscaled 4x, our drawn key, retail Key3
     rows 2-3: the 14 spell-shape glyphs */
/* the item looks the randomizer will use (decided 2026-10-26). kind: 'S' sprite texture, 'M' static mesh,
   'K' skeletal mesh, 'C' an actual class (spawned with its collision off, for looks that need its skins) */
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
};
static const int GRIDROW[] = { 10, 4, 6 };   /* entries per row, in order */
#define NGRID (int)(sizeof GRID / sizeof GRID[0])
static float gGridAt[3];
static wchar gMapLogPath[600];
static int N_FancySpawn2 = -1; static u32 gClsEffects;
static int offUSize = -1;

/* worker, every 2 s on a settled map */
static void grid_poll(const wchar *map) {
    static char buf[4097]; static char lastLine[200]; static int primed; HANDLE f; u32 got = 0, size, hi; char *p, *line = 0;
    char mapA[64]; int i; float v[3];
    if (gGridPending || !GetPrivateProfileIntW(L"Test", L"SpriteGrid", 0, gIniPath)) { primed = 0; return; }
    f = CreateFileW(gMapLogPath, 0x80000000 /*GENERIC_READ*/, 1 | 2 | 4, 0, 3 /*OPEN_EXISTING*/, 0x80, 0);
    if (f == (HANDLE)-1) return;
    size = GetFileSize(f, &hi);
    SetFilePointer(f, size > 4096 ? (int)(size - 4096) : 0, 0, 0);
    ReadFile(f, buf, 4096, &got, 0); CloseHandle(f); buf[got] = 0;
    for (p = buf; *p; p++) if (p[0] == 'M' && p[1] == 'A' && p[2] == 'R' && p[3] == 'K' && p[4] == ' ') line = p;
    if (!line) return;
    { char *e = line; while (*e && *e != '\r' && *e != '\n') e++; if (e - line >= 199) return; *e = 0; }
    if (!primed) { primed = 1; for (i = 0; line[i] && i < 199; i++) lastLine[i] = line[i]; lastLine[i] = 0; return; }   /* old marks don't count */
    for (i = 0; line[i] && line[i] == lastLine[i]; i++) ;
    if (!line[i] && !lastLine[i]) return;
    for (i = 0; line[i] && i < 199; i++) lastLine[i] = line[i]; lastLine[i] = 0;
    /* "MARK  map=HP3_Adv5Shack  state=GState090  pos=(-6325, 21, -973)" */
    p = line; while (*p && !(p[0] == 'm' && p[1] == 'a' && p[2] == 'p' && p[3] == '=')) p++;
    if (!*p) return; p += 4;
    for (i = 0; p[i] && p[i] != ' ' && i < 63; i++) mapA[i] = p[i]; mapA[i] = 0;
    for (i = 0; map[i] && mapA[i] && wlow(map[i]) == wlow((wchar)mapA[i]); i++) ;
    if (map[i] || mapA[i]) { log_line(L"grid: the newest mark is on another map"); return; }
    while (*p && *p != '(') p++;
    for (i = 0; i < 3 && *p; i++) {
        int neg = 0, n = 0; p++;
        while (*p == ' ') p++;
        if (*p == '-') { neg = 1; p++; }
        while (*p >= '0' && *p <= '9') n = n * 10 + (*p++ - '0');
        v[i] = (float)(neg ? -n : n);
        while (*p && *p != ',' && *p != ')') p++;
    }
    if (i < 3) return;
    gGridAt[0] = v[0]; gGridAt[1] = v[1]; gGridAt[2] = v[2];
    if (!gClsEffects) gClsEffects = find_class(L"Effects");
    if (offUSize < 0) offUSize = prop_offset(L"BitmapMaterial", L"USize");
    if (N_FancySpawn2 < 0) N_FancySpawn2 = name_index(L"FancySpawn");
    if (!gLoadObject) { HANDLE core = GetModuleHandleW(L"Core.dll");
        gLoadObject = core ? (loadobj_fn)GetProcAddress(core, "?StaticLoadObject@UObject@@SAPAV1@PAVUClass@@PAV1@PBG2KPAVUPackageMap@@@Z") : 0; }
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
    static wchar b[240]; u32 pc, pawn, obj, cls; void *fn; int i, ok = 0, row = 0, col = 0, inrow = GRIDROW[0];
    pc = gPlayer ? P(gPlayer + offPlayerActor) : 0; pawn = pc ? P(pc + offPawn) : 0;
    if (!pawn || !gClsEffects || !gLoadObject || !gClsTexture || offDrawScale <= 0 || offTexture <= 0 || offStyle2 <= 0
        || N_FancySpawn2 < 0 || !(fn = gFindFunction((void *)pawn, N_FancySpawn2, 0))) { log_line(L"grid: lookups failed"); return; }
    for (i = 0; i < NGRID; i++, col++) {
        const struct grident *g = &GRID[i]; float px;
        if (col == inrow) { row++; col = 0; inrow = GRIDROW[row]; }
        cls = g->kind == 'S' ? gClsTexture : g->kind == 'M' ? gClsStaticMesh : g->kind == 'K' ? gClsMesh : gClsClass;
        obj = cls ? (u32)gLoadObject((void *)cls, 0, g->obj, 0, 0x2002, 0) : 0;
        if (!obj) { wsprintfW(b, L"grid: %s (%s) not found", g->label, g->obj); log_line(b); continue; }
        memset(&parms, 0, sizeof parms);
        parms.type = g->kind == 'C' ? obj : gClsEffects;
        parms.x = gGridAt[0] + (col - (inrow - 1) / 2.0f) * 70.0f; parms.y = gGridAt[1] + (row - 1) * 80.0f; parms.z = gGridAt[2] + 40.0f;
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
