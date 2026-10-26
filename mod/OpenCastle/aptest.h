/*
 * Test of the Archipelago item textures (HP3AP.utx, built by tools/utxwriter.py): with
 *   [Test] APLogos=1
 * in HP3_OpenCastle.ini, every cauldron cake, pumpkin pasty and wizard card lying in the level is drawn as a sprite
 * of one of the three tiers (cakes = progression, pasties = useful, cards = filler) once the map has settled.
 * This only proves the package loads and renders; the real item placement comes later.
 * Needs from opencastle.c: gPEOrig-free (plain field writes + StaticLoadObject), log_line, offsets.
 */
typedef void *(__cdecl *loadobj_fn)(void *cls, void *outer, const wchar *name, const wchar *file, u32 flags, void *sandbox);
static loadobj_fn gLoadObject;
static u32 gClsTexture, gAPTex[3];
static int offDrawScale = -1, offTexture = -1, offStyle2 = -1;   /* Actor.DrawScale / Texture / Style (offDrawType: gates) */
static int N_CauldronCake = -1, N_PumpkinPasty = -1, N_WizardCard = -1;
static u32 gAPPickups[512]; static int gNAPPickups;
static u32 gAPLevel; static int gAPMode;   /* 1 = HP3AP.utx tiers, 2 = a retail texture (HP_FX.Particles.Key1) for all */

/* worker: called once a map has settled */
struct findpu { u32 level; };
static int v_findpu(u32 o, int name, u32 cls, void *c) {
    struct findpu *f = c; (void)name;
    if (gNAPPickups >= 512 || !cls || rd32(o + offLevel) != f->level || !is_a_cls(cls, gClsPickup)) return 0;
    gAPPickups[gNAPPickups++] = o;
    return 0;
}
static void aptest_poll(u32 level) {
    struct findpu f;
    if (gAPTestPending || level == gAPLevel) return;
    if (!(gAPMode = GetPrivateProfileIntW(L"Test", L"APLogos", 0, gIniPath))) return;
    if (!gLoadObject) {
        HANDLE core = GetModuleHandleW(L"Core.dll");
        gLoadObject = core ? (loadobj_fn)GetProcAddress(core, "?StaticLoadObject@UObject@@SAPAV1@PAVUClass@@PAV1@PBG2KPAVUPackageMap@@@Z") : 0;
        gClsTexture = find_class(L"Texture");
        offDrawScale = prop_offset(L"Actor", L"DrawScale"); offTexture = prop_offset(L"Actor", L"Texture");
        offStyle2 = prop_offset(L"Actor", L"Style");
        N_CauldronCake = name_index(L"CauldronCake"); N_PumpkinPasty = name_index(L"PumpkinPasty"); N_WizardCard = name_index(L"WizardCard");
    }
    if (!gLoadObject || !gClsTexture || !gClsPickup || offDrawScale <= 0 || offTexture <= 0 || offStyle2 <= 0 || offDrawType <= 0) {
        log_line(L"aptest: lookups failed, test off"); gAPLevel = level; return;
    }
    gNAPPickups = 0; f.level = level; for_each_object(v_findpu, &f);
    gAPLevel = level; gAPTestPending = 1;
}
/* is `cls` (or a parent) named n? */
static int cls_named(u32 cls, int n) { int g = 64; while (cls && g--) { if ((int)P(cls + 0x20) == n) return 1; cls = P(cls + 0x2C); } return 0; }
/* game thread */
static void aptest_apply(void) {
    if (!gLoadObject) return;
    static const wchar *names[3] = { L"HP3AP.APLogoProgression", L"HP3AP.APLogoUseful", L"HP3AP.APLogoFiller" };
    static wchar b[200]; int i, n[3] = { 0, 0, 0 };
    /* load every time: a texture nothing references is garbage collected at the next level change, so a pointer
       kept from an earlier map is stale (poc6 crashed drawing / cleaning up through one). StaticLoadObject returns the
       object at once when it is already loaded. */
    for (i = 0; i < 3; i++)
        gAPTex[i] = (u32)gLoadObject((void *)gClsTexture, 0, gAPMode == 2 ? L"HP_FX.Particles.Key1" : names[i], 0, 0x2002 /*NoWarn|Quiet*/, 0);
    wsprintfW(b, L"aptest: mode %d", gAPMode); log_line(b);
    wsprintfW(b, L"aptest: HP3AP textures %08x %08x %08x", gAPTex[0], gAPTex[1], gAPTex[2]); log_line(b);
    if (!gAPTex[0] || !gAPTex[1] || !gAPTex[2]) { log_line(L"aptest: HP3AP.utx missing from the textures folder?"); return; }
    for (i = 0; i < gNAPPickups; i++) {
        u32 o = gAPPickups[i], cls = P(o + 0x24); int t;
        if (cls_named(cls, N_CauldronCake)) t = 0; else if (cls_named(cls, N_PumpkinPasty)) t = 1; else if (cls_named(cls, N_WizardCard)) t = 2; else continue;
        *(u8 *)(o + offDrawType) = 1;            /* DT_Sprite */
        *(u32 *)(o + offTexture) = gAPTex[t];
        *(u8 *)(o + offStyle2) = 2;              /* STY_Masked */
        *(float *)(o + offDrawScale) = 0.3f;     /* 128 px -> about 38 units */
        n[t]++;
    }
    wsprintfW(b, L"aptest: drawn as AP sprites: %d cakes (progression), %d pasties (useful), %d cards (filler)", n[0], n[1], n[2]);
    log_line(b);
}

/* game thread, after a KWPickup's PostBeginPlay: goodies thrown out of chests and cauldrons after the map settled
   get the same look. */
static void aptest_spawned(u32 o) {
    static const wchar *names[3] = { L"HP3AP.APLogoProgression", L"HP3AP.APLogoUseful", L"HP3AP.APLogoFiller" };
    u32 cls, tex; int t;
    if (!gAPMode || !gLoadObject || offDrawType <= 0) return;
    cls = P(o + 0x24);
    if (cls_named(cls, N_CauldronCake)) t = 0; else if (cls_named(cls, N_PumpkinPasty)) t = 1; else if (cls_named(cls, N_WizardCard)) t = 2; else return;
    tex = (u32)gLoadObject((void *)gClsTexture, 0, gAPMode == 2 ? L"HP_FX.Particles.Key1" : names[t], 0, 0x2002, 0);
    if (!tex) return;
    *(u8 *)(o + offDrawType) = 1; *(u32 *)(o + offTexture) = tex; *(u8 *)(o + offStyle2) = 2; *(float *)(o + offDrawScale) = 0.3f;
}

/* ---- one-time test: every candidate item sprite in a grid where the player last pressed the MapTracker mark key
   (End). With [Test] SpriteGrid=1 the worker watches HP3_MapLog.txt; each new MARK on the current map spawns the grid
   (inert Engine.Effects actors: no collision, sprite drawn with the texture), 7 columns, 60 units apart:
     row 1: AP progression / useful / filler, keys: retail Key2, Key2 upscaled 4x, our drawn key, retail Key3
     rows 2-3: the 14 spell-shape glyphs */
static const wchar *GRID[] = {
    L"HP3AP.APLogoProgression", L"HP3AP.APLogoUseful", L"HP3AP.APLogoFiller",
    L"HP_FX.Particles.Key2", L"HP3AP.APKeyUpscaled", L"HP3AP.APKeyDrawn", L"HP_FX.Particles.Key3",
    L"SpellShapes.Shapes.SPSalohomora", L"SpellShapes.Shapes.SPSCarpeRetractum", L"SpellShapes.Shapes.SPSdepulso",
    L"SpellShapes.Shapes.SPSdraconifor", L"SpellShapes.Shapes.SPSexpecto", L"SpellShapes.Shapes.SPSglacius",
    L"SpellShapes.Shapes.SPSlapifors",
    L"SpellShapes.Shapes.SPSlumos", L"SpellShapes.Shapes.SPSrictusempra", L"SpellShapes.Shapes.SPSspongify",
    L"SpellShapes.Shapes.SPSavifors", L"SpellShapes.Shapes.SPSdiffindo", L"SpellShapes.Shapes.SPSflipendo",
    L"SpellShapes.Shapes.SPSskurge",
};
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
    if (offDrawScale <= 0) { offDrawScale = prop_offset(L"Actor", L"DrawScale"); offTexture = prop_offset(L"Actor", L"Texture"); offStyle2 = prop_offset(L"Actor", L"Style"); }
    gGridPending = 1;
}
/* game thread */
static void grid_spawn(void) {
    struct { u32 type, owner; int tag; float x, y, z; int p, yw, r; u32 bz; u32 ret; } parms;
    static wchar b[200]; u32 pc, pawn, tex; void *fn; int i, ok = 0;
    pc = gPlayer ? P(gPlayer + offPlayerActor) : 0; pawn = pc ? P(pc + offPawn) : 0;
    if (!pawn || !gClsEffects || !gLoadObject || !gClsTexture || offDrawScale <= 0 || offTexture <= 0 || offStyle2 <= 0
        || N_FancySpawn2 < 0 || !(fn = gFindFunction((void *)pawn, N_FancySpawn2, 0))) { log_line(L"grid: lookups failed"); return; }
    for (i = 0; i < NGRID; i++) {
        int row = i < 7 ? 0 : (i < 14 ? 1 : 2), col = i < 7 ? i : (i - 7) % 7; float px;
        tex = (u32)gLoadObject((void *)gClsTexture, 0, GRID[i], 0, 0x2002, 0);
        if (!tex) { wsprintfW(b, L"grid: %s not found", GRID[i]); log_line(b); continue; }
        memset(&parms, 0, sizeof parms);
        parms.type = gClsEffects;
        parms.x = gGridAt[0] + (col - 3) * 60.0f; parms.y = gGridAt[1] + (row - 1) * 60.0f; parms.z = gGridAt[2] + 40.0f;
        gPEOrig((void *)pawn, fn, &parms, 0);
        if (!parms.ret) { wsprintfW(b, L"grid: spawn for %s failed", GRID[i]); log_line(b); continue; }
        px = offUSize > 0 ? (float)(int)P(tex + offUSize) : 64.0f; if (px <= 0) px = 64.0f;
        *(u32 *)(parms.ret + offTexture) = tex;
        *(u8 *)(parms.ret + offStyle2) = 2;                 /* STY_Masked */
        *(float *)(parms.ret + offDrawScale) = 38.0f / px;  /* every sprite about 38 units across */
        ok++;
    }
    wsprintfW(b, L"grid: %d of %d sprites placed around (%d, %d, %d)", ok, NGRID, (int)gGridAt[0], (int)gGridAt[1], (int)gGridAt[2]);
    log_line(b);
}
