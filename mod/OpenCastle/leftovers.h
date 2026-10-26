/*
 * Leftover goodies: items a chest/cauldron/crate threw out that the player didn't pick up survive a level change.
 *
 * The game keeps a spawner's Lives in the PersistentStore (<id><name>_lives), so an opened chest stays opened, but
 * the goodies it threw out are dynamic actors that nothing records. Everything here runs on the game thread, from
 * the ProcessEvent hook:
 *   PostBeginPlay (spawner)            remember it: name, location, its 8 GoodieToSpawn classes, Lives at load
 *   PostPersistentDataRestored (sp.)   Lives at load (after the store restored it); read our note <id><name>_OCleft,
 *                                      clear it and queue the leftovers to be thrown out again
 *   PostBeginPlay (KWPickup)           link a new pickup to the nearest spawner opened this visit whose goodie list
 *                                      holds the pickup's class (or to the spawner being refilled)
 *   Destroyed (pickup)                 HasBeenPickedUp=1 -> collected; otherwise lost (faded / fell out of the
 *                                      world) and it stays a leftover
 *   Destroyed (spawner)                destroyable crates: keep the record, forget the object
 *   TravelExporting (PersistentStore)  the store is about to travel to the next level (the old level is still
 *                                      loaded): write a note for every spawner with leftovers: goodie indices "0,0,3".
 *                                      (SavePersistentData can't be used: ServerTravel calls it from script, which
 *                                      doesn't pass through ProcessEvent.)
 * The queued leftovers are spawned (FancySpawn on the player's pawn, in a ring above the spawner) once the worker
 * says the map has settled.
 * Needs from opencastle.c: gPEOrig, gFindFunction, log_line, offsets, gPlayer.
 */
#define LO_MAXSP 256
#define LO_MAXPU 512
#define LO_LINK_R 400.0f
struct lo_sp { u32 obj, level; int name; float x, y, z; u32 goodies[8]; int lives0; int dead; };
struct lo_pu { u32 obj; int sp, idx, state; };   /* state 0 on the ground, 1 collected, 2 lost */
static struct lo_sp gSp[LO_MAXSP]; static int gNSp;
static struct lo_pu gPu[LO_MAXPU]; static int gNPu;
static u32 gLoLevel, gLoFlushed;
static int gForceSp = -1, gForceIdx = -1;
struct lo_q { u32 cls; float x, y, z; int sp, idx; };
static struct lo_q gQ[128]; static int gNQ; static u32 gQLevel;
static volatile int gPendingRespawn;

static u32 gClsSpawner, gClsPickup;
static int offLives = -1, offGoodies = -1, offPicked = -1, offStore = -1;   /* GameInfo.PersistentStore */
static int N_PostBeginPlay = -1, N_Destroyed = -1, N_TravelExporting = -1, N_PostPDR = -1, N_SaveValue = -1, N_RestoreValue = -1,
           N_FancySpawn = -1;

static u32 P(u32 a) { return *(u32 *)a; }   /* game thread: the objects are live */
static int is_a_fast(u32 obj, u32 want) { u32 c = P(obj + 0x24); int g = 64; while (c && g--) { if (c == want) return 1; c = P(c + 0x2C); } return 0; }

static u32 cur_level(void) {   /* the level the player is on now */
    u32 pc = gPlayer && offPlayerActor >= 0 ? P(gPlayer + offPlayerActor) : 0;
    return pc && offLevel >= 0 ? P(pc + offLevel) : 0;
}
static void lo_reset(u32 level) { gNSp = 0; gNPu = 0; gLoLevel = level; gLoFlushed = 0; }
static int lo_find_sp(u32 obj) { int i; for (i = 0; i < gNSp; i++) if (gSp[i].obj == obj) return i; return -1; }

/* key <LevelID><SpawnerName>_OCleft into an FString-ready buffer */
static int lo_key(int sp, wchar *out, int max) {
    wchar id[32], nm[64];
    if (offID < 0 || !read_fstring(gLoLevel + offID, id, 32) || !name_of(gSp[sp].name, nm, 64)) return 0;
    wsprintfW(out, L"%s%s_OCleft", id, nm); (void)max;
    return 1;
}
static u32 lo_store(void) {
    u32 game = offGame > 0 ? P(gLoLevel + offGame) : 0;
    return game && offStore > 0 ? P(game + offStore) : 0;
}
static void lo_save_value(u32 store, wchar *key, wchar *val) {
    struct { wchar *k; int kn, km; wchar *v; int vn, vm; } parms; void *fn;
    if (!store || N_SaveValue < 0 || !(fn = gFindFunction((void *)store, N_SaveValue, 0))) return;
    parms.k = key; parms.kn = parms.km = wlen(key) + 1;
    parms.v = val; parms.vn = parms.vm = wlen(val) + 1;
    gPEOrig((void *)store, fn, &parms, 0);
}
/* returns 1 and fills val if the key exists. The out string is allocated by the engine; we copy it and leave it be. */
static int lo_restore_value(u32 store, wchar *key, wchar *val, int max) {
    struct { wchar *k; int kn, km; wchar *v; int vn, vm; u32 ret; } parms; void *fn; int i;
    val[0] = 0;
    if (!store || N_RestoreValue < 0 || !(fn = gFindFunction((void *)store, N_RestoreValue, 0))) return 0;
    parms.k = key; parms.kn = parms.km = wlen(key) + 1;
    parms.v = 0; parms.vn = parms.vm = 0; parms.ret = 0;
    gPEOrig((void *)store, fn, &parms, 0);
    if (!(parms.ret & 1) || !parms.v || parms.vn <= 0) return 0;
    for (i = 0; i < max - 1 && i < parms.vn && parms.v[i]; i++) val[i] = parms.v[i];
    val[i] = 0;
    return 1;
}

static void lo_add_spawner(u32 obj) {
    u32 level = P(obj + offLevel); float *loc = (float *)(obj + offLoc); int k; struct lo_sp *s;
    if (level != gLoLevel) lo_reset(level);
    if (lo_find_sp(obj) >= 0 || gNSp >= LO_MAXSP) return;
    s = &gSp[gNSp++];
    s->obj = obj; s->level = level; s->name = (int)P(obj + 0x20); s->x = loc[0]; s->y = loc[1]; s->z = loc[2];
    for (k = 0; k < 8; k++) s->goodies[k] = P(obj + offGoodies + 4 * k);
    s->lives0 = (int)P(obj + offLives); s->dead = 0;
}

static void lo_link_pickup(u32 obj) {
    u32 cls = P(obj + 0x24); float *loc = (float *)(obj + offLoc); int i, k, best = -1, bidx = -1; float bd = LO_LINK_R * LO_LINK_R;
    if (P(obj + offLevel) != gLoLevel || gNPu >= LO_MAXPU) return;
    if (gForceSp >= 0) { best = gForceSp; bidx = gForceIdx; }
    else for (i = 0; i < gNSp; i++) {
        struct lo_sp *s = &gSp[i]; float dx = loc[0] - s->x, dy = loc[1] - s->y, dz = loc[2] - s->z, d2 = dx * dx + dy * dy + dz * dz;
        int opened = s->dead || (s->obj && (int)P(s->obj + offLives) < s->lives0);
        if (!opened || d2 >= bd) continue;
        for (k = 0; k < 8; k++) if (s->goodies[k] == cls) break;
        if (k == 8) continue;
        bd = d2; best = i; bidx = k;
    }
    if (best < 0) return;
    gPu[gNPu].obj = obj; gPu[gNPu].sp = best; gPu[gNPu].idx = bidx; gPu[gNPu].state = 0; gNPu++;
}

static void lo_destroyed(u32 obj) {
    int i;
    for (i = 0; i < gNPu; i++) if (gPu[i].obj == obj && gPu[i].state == 0) {
        gPu[i].state = (offPicked >= 0 && P(obj + offPicked) == 1) ? 1 : 2;
        gPu[i].obj = 0;
        return;
    }
    if ((i = lo_find_sp(obj)) >= 0) { gSp[i].obj = 0; gSp[i].dead = 1; }
}

/* the level is being left: write a note for every spawner with goodies still out */
static void lo_flush(void) {
    static wchar key[160], val[400], msg[300]; u32 store = lo_store(); int s, i, n, total = 0, notes = 0;
    if (!store) { log_line(L"leftovers: no PersistentStore, nothing saved"); return; }
    for (s = 0; s < gNSp; s++) {
        n = 0; val[0] = 0;
        for (i = 0; i < gNPu; i++) if (gPu[i].sp == s && gPu[i].state != 1 && n < 60) {
            wsprintfW(val + wlen(val), n ? L",%d" : L"%d", gPu[i].idx); n++;
        }
        if (!n || !lo_key(s, key, 160)) continue;
        lo_save_value(store, key, val);
        wsprintfW(msg, L"leftovers: saved %s = %s", key, val); log_line(msg);
        total += n; notes++;
    }
    wsprintfW(msg, L"leftovers: leaving with %d item(s) on the ground from %d spawner(s) (%d spawners known, %d pickups tracked)",
              total, notes, gNSp, gNPu);
    log_line(msg);
}

/* a spawner's persistent data is back: pick up our note */
static void lo_restore(u32 obj) {
    static wchar key[160], val[400], msg[300]; u32 store; int s, i;
    if ((s = lo_find_sp(obj)) < 0) { lo_add_spawner(obj); if ((s = lo_find_sp(obj)) < 0) return; }
    gSp[s].lives0 = (int)P(obj + offLives);
    if (!(store = lo_store()) || !lo_key(s, key, 160) || !lo_restore_value(store, key, val, 400) || !val[0]) return;
    if (gQLevel != gLoLevel) { gNQ = 0; gQLevel = gLoLevel; }
    for (i = 0; val[i]; ) {
        int idx = 0, ring;
        while (val[i] >= '0' && val[i] <= '9') idx = idx * 10 + (val[i++] - '0');
        if (val[i] == ',') i++;
        if (idx < 0 || idx > 7 || !gSp[s].goodies[idx] || gNQ >= 128) continue;
        ring = gNQ % 8;
        gQ[gNQ].cls = gSp[s].goodies[idx]; gQ[gNQ].sp = s; gQ[gNQ].idx = idx;
        gQ[gNQ].x = gSp[s].x + (ring == 0 ? 0 : ring < 4 ? 60.0f : -60.0f) * (ring & 1 ? 1.0f : 0.5f);
        gQ[gNQ].y = gSp[s].y + (ring == 0 ? 0 : (ring & 2) ? 60.0f : -60.0f) * (ring & 1 ? 0.5f : 1.0f);
        gQ[gNQ].z = gSp[s].z + 50.0f;
        gNQ++;
    }
    val[0] = 0; lo_save_value(store, key, val);   /* cleared; whatever is left out again gets a new note */
    wsprintfW(msg, L"leftovers: %s has items to throw out again (%d queued)", key, gNQ); log_line(msg);
}

/* the map has settled: throw the queued leftovers out */
static void lo_respawn(void) {
    struct { u32 type, owner; int tag; float x, y, z; int p, yw, r; u32 bz; u32 ret; } parms;
    u32 pc, pawn; void *fn; int i, ok = 0; static wchar msg[200];
    if (!gNQ || gQLevel != gLoLevel || gLoLevel != cur_level()) { gNQ = 0; return; }
    pc = gPlayer ? P(gPlayer + offPlayerActor) : 0; pawn = pc ? P(pc + offPawn) : 0;
    if (!pawn || N_FancySpawn < 0 || !(fn = gFindFunction((void *)pawn, N_FancySpawn, 0))) { log_line(L"leftovers: FancySpawn not found"); gNQ = 0; return; }
    for (i = 0; i < gNQ; i++) {
        memset(&parms, 0, sizeof parms);
        parms.type = gQ[i].cls; parms.x = gQ[i].x; parms.y = gQ[i].y; parms.z = gQ[i].z;
        gForceSp = gQ[i].sp; gForceIdx = gQ[i].idx;
        gPEOrig((void *)pawn, fn, &parms, 0);
        gForceSp = gForceIdx = -1;
        if (parms.ret) ok++;
    }
    wsprintfW(msg, L"leftovers: threw out %d of %d item(s) again", ok, gNQ); log_line(msg);
    gNQ = 0;
}

/* called from pe_hook before the original runs; returns nothing, never blocks the event */
static void lo_event(u32 self, int fname) {
    if (!gClsSpawner || !gClsPickup || offLevel <= 0 || offLoc <= 0 || offLives <= 0 || offGoodies <= 0 || offStore <= 0) return;
    if (fname == N_PostBeginPlay) {
        if (is_a_fast(self, gClsSpawner)) lo_add_spawner(self);
        else if (is_a_fast(self, gClsPickup)) lo_link_pickup(self);
    } else if (fname == N_Destroyed) {
        if (P(self + offLevel) == gLoLevel) lo_destroyed(self);
    } else if (fname == N_PostPDR) {
        if (is_a_fast(self, gClsSpawner)) { if (P(self + offLevel) != gLoLevel) lo_reset(P(self + offLevel)); lo_restore(self); }
    } else if (fname == N_TravelExporting) {
        (void)self;
        if (gLoLevel && gLoLevel == cur_level() && gLoFlushed != gLoLevel) { gLoFlushed = gLoLevel; lo_flush(); }
    }
}
