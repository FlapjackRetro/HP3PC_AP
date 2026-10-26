/*
 * Blockers: for a locked challenge that has no door of its own, spawn one. A portcullis static mesh (the castle's
 * own, HP3_InterArch_SM.Movers.Portcullis_SM, 128 x 204 units) is placed in an existing arch, with a blocking
 * collision cylinder, on an inert Engine.Effects actor (SetCollisionSize + SetCollision through ProcessEvent).
 * While the key is locked it's there; when it unlocks it's destroyed. Spawned again on every visit while locked.
 */
struct blocker { const wchar *map, *key, *why; float x, y, z; int yaw; const wchar *mesh; float scale, radius, height; };
static const struct blocker BLOCKERS[] = {
    /* GroundsHub, the COMC arch (StaticMeshActor260, COMCgate_SM at (-1868, 992, 576), opening centred under its two
       tapestries). The path behind it leads to the COMC walk-in, and past that out of bounds. */
    { L"HP3_GroundsHub", L"Hippogriff", L"COMC arch portcullis", -1868, 992, 576, 0,
      L"HP3_InterArch_SM.Movers.Portcullis_SM", 2.0f, 135.0f, 210.0f },
};
#define NBLOCKERS (int)(sizeof BLOCKERS / sizeof BLOCKERS[0])
static u32 gBlockObj[NBLOCKERS], gBlockLevel[NBLOCKERS];
static int N_SetCollisionSize = -1, N_Destroy = -1;

/* worker, from gates_poll: decide */
static void blockers_poll(u32 level, const wchar *map) {
    int i, k;
    static int looked;
    if (!looked) {   /* shared with the look grid (aptest.h) */
        looked = 1;
        N_SetCollisionSize = name_index(L"SetCollisionSize"); N_Destroy = name_index(L"Destroy");
        if (N_SetCollision < 0) N_SetCollision = name_index(L"SetCollision");
        if (N_FancySpawn2 < 0) N_FancySpawn2 = name_index(L"FancySpawn");
        if (!gClsEffects) gClsEffects = find_class(L"Effects");
        if (!gClsStaticMesh) gClsStaticMesh = find_class(L"StaticMesh");
        if (offStaticMesh < 0) offStaticMesh = prop_offset(L"Actor", L"StaticMesh");
        if (offDrawScale <= 0) { offDrawScale = prop_offset(L"Actor", L"DrawScale"); offTexture = prop_offset(L"Actor", L"Texture"); offStyle2 = prop_offset(L"Actor", L"Style"); }
    }
    for (i = 0; i < NBLOCKERS; i++) {
        int on = 1, locked;
        if (gBlockLevel[i] != level) { gBlockObj[i] = 0; gBlockLevel[i] = level; }   /* a new map: the old actor is gone */
        if (!weq(BLOCKERS[i].map, map)) continue;
        for (k = 0; k < NDOORS; k++) if (weq(DOORS[k].key, BLOCKERS[i].key)) { on = gDoorOn[k]; break; }
        locked = !on;
        if (locked && !gBlockObj[i]) gBlockSpawn |= 1u << i;
        if (!locked && gBlockObj[i]) gBlockKill |= 1u << i;
    }
}
/* game thread */
static void blockers_apply(void) {
    struct { u32 type, owner; int tag; float x, y, z; int p, yw, r; u32 bz; u32 ret; } parms;
    static wchar b[200]; u32 spawn = gBlockSpawn, kill = gBlockKill, pc, pawn, mesh; void *fn; int i;
    gBlockSpawn = gBlockKill = 0;
    for (i = 0; i < NBLOCKERS; i++) if (((kill >> i) & 1) && gBlockObj[i]) {
        void *d = N_Destroy >= 0 ? gFindFunction((void *)gBlockObj[i], N_Destroy, 0) : 0;
        struct { u32 ret; } dp = { 0 };
        if (d) gPEOrig((void *)gBlockObj[i], d, &dp, 0);
        wsprintfW(b, L"blocker %s: removed (%s unlocked)", BLOCKERS[i].why, BLOCKERS[i].key); log_line(b);
        gBlockObj[i] = 0;
    }
    if (!spawn) return;
    pc = gPlayer ? P(gPlayer + offPlayerActor) : 0; pawn = pc ? P(pc + offPawn) : 0;
    if (!gLoadObject) { HANDLE core = GetModuleHandleW(L"Core.dll");
        gLoadObject = core ? (loadobj_fn)GetProcAddress(core, "?StaticLoadObject@UObject@@SAPAV1@PAVUClass@@PAV1@PBG2KPAVUPackageMap@@@Z") : 0; }
    if (!pawn || !gLoadObject || !gClsEffects || !gClsStaticMesh || offStaticMesh <= 0 || offDrawScale <= 0 || offDrawType <= 0
        || N_FancySpawn2 < 0 || !(fn = gFindFunction((void *)pawn, N_FancySpawn2, 0))) { log_line(L"blocker: lookups not ready"); return; }
    for (i = 0; i < NBLOCKERS; i++) if (((spawn >> i) & 1) && !gBlockObj[i]) {
        const struct blocker *bl = &BLOCKERS[i];
        void *sz, *sc;
        mesh = (u32)gLoadObject((void *)gClsStaticMesh, 0, bl->mesh, 0, 0x2002, 0);
        if (!mesh) { wsprintfW(b, L"blocker %s: mesh %s not found", bl->why, bl->mesh); log_line(b); continue; }
        memset(&parms, 0, sizeof parms);
        parms.type = gClsEffects; parms.x = bl->x; parms.y = bl->y; parms.z = bl->z; parms.yw = bl->yaw;
        gPEOrig((void *)pawn, fn, &parms, 0);
        if (!parms.ret) { wsprintfW(b, L"blocker %s: spawn failed", bl->why); log_line(b); continue; }
        *(u32 *)(parms.ret + offStaticMesh) = mesh; *(u8 *)(parms.ret + offDrawType) = 8; *(float *)(parms.ret + offDrawScale) = bl->scale;
        if (N_SetCollisionSize >= 0 && (sz = gFindFunction((void *)parms.ret, N_SetCollisionSize, 0))) {
            struct { float r, h, w; u32 ret; } sp = { bl->radius, bl->height, 0, 0 };
            gPEOrig((void *)parms.ret, sz, &sp, 0);
        }
        if (N_SetCollision >= 0 && (sc = gFindFunction((void *)parms.ret, N_SetCollision, 0))) {
            struct { u32 a, b, c; } cp = { 1, 1, 1 };
            gPEOrig((void *)parms.ret, sc, &cp, 0);
        }
        gBlockObj[i] = parms.ret;
        wsprintfW(b, L"blocker %s: placed (%s locked)", bl->why, bl->key); log_line(b);
    }
}
