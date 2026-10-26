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
    static wchar b[200]; int i, n[3] = { 0, 0, 0 }; static int loadedMode;
    if (loadedMode != gAPMode) { gAPTex[0] = gAPTex[1] = gAPTex[2] = 0; loadedMode = gAPMode; }
    for (i = 0; i < 3; i++) if (!gAPTex[i])
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
