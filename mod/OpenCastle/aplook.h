/*
 * Item looks: every location's pickup shows the item the multiworld put there (docs/ITEM_MODELS.md). The client
 * scouts all locations once connected and sends LOOKS <loc>:<code>,... (code table below, the same in
 * apworld/hp3pc/client.py). HP3 items show their own art; other games' items an Archipelago logo by tier.
 *
 * Only the vanilla pickup's drawing changes (DrawType / Texture / StaticMesh / Mesh / DrawScale): it is still the
 * same pickup, picked up and checked as before (ap.h). Pickups grow in as the player comes near (KWPickup sets
 * DrawScale = Default.DrawScale * PickupVisibility / 5), so the look's own scale is put back every tick with the
 * same fraction.
 *   placed pasties / cakes / cards, doxy reward cards   once the map has settled (ap.h's map scan)
 *   event spawns, a container's first goodie            as they appear
 * Shops, vendors and secrets have no pickup to change.
 */
#include "wc_textures.h"
/* codes 100 + n: wizard card n (WizardCardManager.Cards index), the card mesh with that card's face (wc_textures.h) */
#define LOOK_CARD0 100
struct lookdef { unsigned char code; char kind; const wchar *obj; float scale; int anim; };
/* kind: S sprite texture (scale = size in units), M static mesh, K skeletal mesh, A skeletal mesh with a looping idle
   (anim = aptest.h ANIM_TRY row) */
static const struct lookdef LOOKDEFS[] = {
    { 1, 'S', L"SpellShapes.Shapes.SPSalohomora", 38, 0 },        { 2, 'S', L"SpellShapes.Shapes.SPSCarpeRetractum", 38, 0 },
    { 3, 'S', L"SpellShapes.Shapes.SPSdepulso", 38, 0 },          { 4, 'S', L"SpellShapes.Shapes.SPSdraconifor", 38, 0 },
    { 5, 'S', L"SpellShapes.Shapes.SPSexpecto", 38, 0 },          { 6, 'S', L"SpellShapes.Shapes.SPSglacius", 38, 0 },
    { 7, 'S', L"SpellShapes.Shapes.SPSlapifors", 38, 0 },         { 8, 'S', L"SpellShapes.Shapes.SPSlumos", 38, 0 },
    { 9, 'S', L"SpellShapes.Shapes.SPSrictusempra", 38, 0 },      { 10, 'S', L"SpellShapes.Shapes.SPSspongify", 38, 0 },
    { 20, 'S', L"HP3AP.APKeyDrawn", 38, 0 },                      /* level unlocks */
    { 21, 'A', L"HPCharacters.Buckbeak", 0.30f, 0 },               /* Hippogriff Challenge Unlock */
    { 22, 'A', L"HPCharacters.Pixie", 0.65f, 1 },                  /* Doxy Challenge Unlock */
    { 23, 'A', L"HPCharacters.MBOM", 0.8f, 2 },                    /* Monster Book Challenge Unlock */
    { 30, 'M', L"HP3_InteriorAccents_SM.books.BlueBandScroll_SM", 1.0f, 0 },   /* portrait / shortcut passwords */
    { 32, 'K', L"HProps.MarauderMap", 1.0f, 0 },
    { 33, 'M', L"HP3_GamePlay_sm.Spells_Objects.PumpkinPasty_SM", 2.0f, 0 },
    { 34, 'M', L"HP3_GamePlay_sm.Spells_Objects.CauldronCake_SM", 2.5f, 0 },
    { 35, 'A', L"HPCharacters.ChocolateFrog", 1.25f, 3 },   /* needs an animation: in its reference pose it lies splayed */
    { 36, 'M', L"HP3_GamePlay_sm.WizardCards.WizardCard_SM", 1.0f, 0 },
    { 40, 'S', L"HP3AP.APLogoProgression", 38, 0 },               /* other games: progression */
    { 41, 'S', L"HP3AP.APLogoUseful", 38, 0 },                    /* useful */
    { 42, 'S', L"HP3AP.APLogoFiller", 38, 0 },                    /* filler (and traps) */
};
#define NLOOKDEFS (int)(sizeof LOOKDEFS / sizeof LOOKDEFS[0])
static u32 gLookObj[NLOOKDEFS]; static unsigned char gLookTried[NLOOKDEFS];

#define LOOK_MAX 192
struct lookact { u32 obj; short li; unsigned char code, done; float base; };
static struct lookact gLA[LOOK_MAX]; static int gNLA; static u32 gLALevel; static int gLAGen = -1, gLAScanned;
static int offPUVis = -1, offSkins = -1;
typedef int (__thiscall *addzeroed_fn)(void *array, int elemsize, int count);   /* Core.dll FArray::AddZeroed(ElementSize, Count) */
static addzeroed_fn gAddZeroed; static u32 gClsMaterial, gCardTex[NWC_TEXTURES]; static unsigned char gCardTexTried[NWC_TEXTURES];

static void ap_look_queue(u32 obj, int li) {   /* game thread */
    int i;
    if (cur_level() != gLALevel) { gNLA = 0; gLALevel = cur_level(); gLAScanned = 0; }
    for (i = 0; i < gNLA; i++) if (gLA[i].obj == obj) return;
    if (gNLA >= LOOK_MAX) return;
    gLA[gNLA].obj = obj; gLA[gNLA].li = (short)li; gLA[gNLA].code = 0; gLA[gNLA].done = 0; gLA[gNLA].base = 0; gNLA++;
}
static void ap_look_forget(u32 obj) { int i; for (i = 0; i < gNLA; i++) if (gLA[i].obj == obj) gLA[i].obj = 0; }

static int look_ready(void) {   /* the lookups the grid also uses (aptest.h); retried every 2 s until they're all in */
    static int state, tries; static u32 last; static wchar b[300];
    if (state > 0) return 1;
    if (last && GetTickCount() - last < 2000) return 0;
    last = GetTickCount();
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
    if (offStaticMesh <= 0) offStaticMesh = prop_offset(L"Actor", L"StaticMesh");
    if (offMesh <= 0) offMesh = prop_offset(L"Actor", L"Mesh");
    if (offDrawScale <= 0) offDrawScale = prop_offset(L"Actor", L"DrawScale");
    if (offTexture <= 0) offTexture = prop_offset(L"Actor", L"Texture");
    if (offStyle2 <= 0) offStyle2 = prop_offset(L"Actor", L"Style");
    if (offDrawType <= 0) offDrawType = prop_offset(L"Actor", L"DrawType");
    if (offUSize <= 0) offUSize = prop_offset(L"BitmapMaterial", L"USize");
    if (offPUVis <= 0) offPUVis = prop_offset(L"KWPickup", L"PickupVisibility");
    if (offSkins <= 0) offSkins = prop_offset(L"Actor", L"Skins");
    if (!gClsMaterial) gClsMaterial = find_class(L"Material");
    if (!gAddZeroed) { HANDLE core = GetModuleHandleW(L"Core.dll"); gAddZeroed = core ? (addzeroed_fn)GetProcAddress(core, "?AddZeroed@FArray@@QAEHHH@Z") : 0; }
    state = (gLoadObject && gClsTexture && gClsStaticMesh && gClsMesh && offStaticMesh > 0 && offMesh > 0 && offDrawScale > 0
             && offTexture > 0 && offStyle2 > 0 && offDrawType > 0) ? 1 : 0;
    if (state > 0) log_line(L"looks: ready");
    else if (++tries == 15) {   /* 30 s on a settled map and still missing: say which */
        wsprintfW(b, L"looks: still waiting (LoadObject %d, classes %d/%d/%d, staticmesh %d mesh %d drawscale %d texture %d style %d drawtype %d)",
                  gLoadObject != 0, gClsTexture != 0, gClsStaticMesh != 0, gClsMesh != 0, offStaticMesh, offMesh, offDrawScale,
                  offTexture, offStyle2, offDrawType);
        log_line(b);
    }
    return state > 0;
}
static int look_def(int code) { int i; for (i = 0; i < NLOOKDEFS; i++) if (LOOKDEFS[i].code == code) return i; return -1; }
static u32 look_asset(int d) {
    const struct lookdef *l = &LOOKDEFS[d]; u32 cls;
    if (gLookObj[d] || gLookTried[d]) return gLookObj[d];
    gLookTried[d] = 1;
    cls = l->kind == 'S' ? gClsTexture : l->kind == 'M' ? gClsStaticMesh : gClsMesh;
    gLookObj[d] = (u32)gLoadObject((void *)cls, 0, l->obj, 0, 0x2002, 0);
    if (!gLookObj[d]) { static wchar b[160]; wsprintfW(b, L"looks: %s not found", l->obj); log_line(b); }
    return gLookObj[d];
}
/* game thread: draw `a` as look `code`; returns the base DrawScale (0 = not changed) */
static float look_apply(u32 a, int code) {
    int d, card = code >= LOOK_CARD0 ? code - LOOK_CARD0 : -1; u32 obj; const struct lookdef *l; float scale;
    if (card >= 0) {   /* a particular wizard card: the card mesh, with that card's face as Skins[0] */
        u32 tex;
        if (card >= NWC_TEXTURES) card = -1;
        else {
            if (!gCardTex[card] && !gCardTexTried[card] && gClsMaterial) { gCardTexTried[card] = 1;
                gCardTex[card] = (u32)gLoadObject((void *)gClsMaterial, 0, WC_TEXTURES[card], 0, 0x2002, 0);
                if (!gCardTex[card]) { static wchar b[160]; wsprintfW(b, L"looks: %s not found", WC_TEXTURES[card]); log_line(b); } }
            tex = gCardTex[card];
            if (tex && offSkins > 0 && (*(int *)(a + offSkins + 4) > 0 || (gAddZeroed && gAddZeroed((void *)(a + offSkins), 4, 1) >= 0 && *(int *)(a + offSkins + 4) == 1)))
                *(u32 *)P(a + offSkins) = tex;   /* element 0 (a card pickup already has one; others get one from the engine:
                                                    AddZeroed(ElementSize 4, Count 1); poc9g had the two swapped, so a
                                                    cake got 4 entries, 3 of them garbage, and level cleanup crashed) */
        }
        code = 36;   /* the card mesh */
    }
    d = look_def(code);
    if (d < 0 || !(obj = look_asset(d))) return 0;
    l = &LOOKDEFS[d];
    /* the pickup's own texture overrides (Actor.Skins: data, count, max) would paint the new mesh (the frog came out
       in the pasty's colours, poc9e): count 0 so the mesh's own materials show */
    if (card < 0 && l->kind != 'S' && offSkins > 0) *(int *)(a + offSkins + 4) = 0;
    if (l->kind == 'S') {
        float px = offUSize > 0 ? (float)(int)P(obj + offUSize) : 64.0f; if (px <= 0) px = 64.0f;
        *(u32 *)(a + offTexture) = obj; *(u8 *)(a + offStyle2) = 2; *(u8 *)(a + offDrawType) = 1;   /* DT_Sprite, STY_Masked */
        scale = l->scale / px;
    } else if (l->kind == 'M') {
        *(u32 *)(a + offStaticMesh) = obj; *(u8 *)(a + offDrawType) = 8; scale = l->scale;
    } else {
        *(u32 *)(a + offMesh) = obj; *(u8 *)(a + offDrawType) = 2; scale = l->scale;
        if (l->kind == 'A') grid_animate(a, obj, l->anim, l->obj);
    }
    *(float *)(a + offDrawScale) = scale;
    return scale;
}
/* game thread, every safe tick */
static void ap_looks_tick(void) {
    u32 level = cur_level(); int i;
    if (!level) return;
    if (level != gLALevel) { gNLA = 0; gLALevel = level; gLAScanned = 0; }
    if (!look_ready()) return;
    /* the map's placed pickups, once its scan is in */
    if (!gLAScanned && gApMapLevel == level) {
        gLAScanned = 1;
        for (i = 0; i < gApMapN; i++) {
            char k = AP_LOCS[gApMapLoc[i]].kind;
            if (gApMapObj[i] && (k == 'P' || k == 'K' || k == 'M') && !gLocChecked[gApMapLoc[i]]) ap_look_queue(gApMapObj[i], gApMapLoc[i]);
        }
    }
    if (gLAGen != gApLookGen) { gLAGen = gApLookGen; for (i = 0; i < gNLA; i++) gLA[i].done = 0; }   /* new looks: redo */
    for (i = 0; i < gNLA; i++) {
        struct lookact *e = &gLA[i]; int code;
        if (!e->obj) continue;
        code = gLook[e->li];
        if (!e->done || code != e->code) {
            e->done = 1; e->code = (unsigned char)code;
            if (code && (e->base = look_apply(e->obj, code)) > 0) {
                static wchar b[200]; wsprintfW(b, L"looks: %s shows look %d", AP_LOCS[e->li].name, code); log_line(b);
            }
        }
        if (e->base > 0) {   /* keep the grow-in, at our scale */
            int vis = offPUVis > 0 ? (int)P(e->obj + offPUVis) : 5;
            if (vis < 1) vis = 1; if (vis > 5) vis = 5;
            *(float *)(e->obj + offDrawScale) = e->base * (float)vis / 5.0f;
        }
    }
}
