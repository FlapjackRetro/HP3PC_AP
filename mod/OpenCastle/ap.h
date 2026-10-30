/*
 * Archipelago: items in, checks out, over the local connection (net.h) to the client (apworld/hp3pc/client.py).
 * Design: docs/CLIENT_SYNC.md. IDs: aptables.h, generated from the apworld (tools/build_mod_tables.py).
 *
 * Protocol (one line each, ASCII; offsets are AP ids minus AP_BASE_ID):
 *   mod -> client   HELLO <mod version>
 *                   BIND <seed>|<slot>      the loaded save's room (BIND - for a save not bound yet)
 *                   CHECK <loc>             a location was just checked
 *                   CHECKS <loc>,<loc>...   every location checked in this save (after each BIND, in chunks)
 *                   MISMATCH                the save belongs to another room: nothing is sent or applied
 *                   DOOR                    the Final Day door has opened (enough cards received)
 *                   GOAL                    the player walked through the Final Day door (sent again on every
 *                                           connection while the save has OC_AP_Goal)
 *   client -> mod   ROOM <seed>|<slot>|<split 0/1>|<cheap 0/1>|<cards required>|<secretsanity 0/1>|<candysanity 0/1>
 *                   SHOPTEXT <loc>|<item name>|<description>   for the shop dialog (apshop.h)
 *                   (the last two are 1 when missing: older clients)
 *                   ITEMS <first index> <item>,<item>...   the full received list, in chunks
 *                   SYNCED <count>          end of the full list
 *                   ITEM <index> <item>     one more received item
 *                   CHECKED <loc>,<loc>...  locations the server already has
 *                   MSG <text>              logged
 *
 * The save (PersistentStore, written into save games) holds, as in CLIENT_SYNC.md:
 *   OC_AP_Bind     "<seed>|<slot>": the room this save belongs to (bound on first connect)
 *   OC_AP_Recv     how many received items the counted path has applied (currency, frogs)
 *   OC_AP_Checked  every location checked in this save, "12,40,..."
 * Idempotent items (spells, doors, passwords, the map, shortcuts, cards) are asserted from the whole received list on
 * every settled map load and whenever items arrive; counted items only from OC_AP_Recv on.
 *
 * Goal (final_day_cards): once the received wizard cards reach the room's cards_required, the Final Day door on the
 * grounds (GroundsHub Mover11, Tag FinalDayDoor) is opened, the way vanilla's GState100 level start does
 * (`Trigger FinalDayDoor`); walking in past it (where vanilla's Final Day cutscene triggers stand) is the goal:
 * OC_AP_Goal is saved, GOAL goes to the client, and the player travels to HP3_Final as vanilla's "Yes" did.
 *
 * Checks (game thread, from the ProcessEvent hook):
 *   P / K with a save key   the pickup actor of that name is picked up (KWPickup.Destroyed with HasBeenPickedUp 1)
 *   E / K without a key     a pickup that appears next to the location's spawn point (HPSpawnThingy / SpawnThingy),
 *                           then is picked up
 *   C                       the container throws out its first goodie (leftovers.h links goodies to spawners)
 *   S                       SecretAreaMarker.bFound (worker poll)
 *   M                       the doxy round's reward card is picked up (WinCard1-5 -> the five cards' actors)
 * A location's pickup gives nothing when touched (apshop.h, ap_touch: the vanilla Touch is replaced, so no inventory
 * and no HUD counter); for a pickup taken some other way, the vanilla reward is taken back: a container's pasty / cake / card goodies are destroyed as they appear; a picked
 * pasty or cake is subtracted again (KWPawn.SubtractFromInventoryCollection) and a picked card goes back to its
 * status before (unless it was received). Shops and vendors: apshop.h.
 */
#include "aptables.h"

#define AP_RECVMAX 4096
static unsigned short gRecv[AP_RECVMAX]; static volatile int gRecvN, gRecvSynced;   /* written by the worker */
static char gApRoom[128]; static volatile int gApRoomGen;                            /* "<seed>|<slot>" */
static volatile int gApSplit, gApCards;   /* gApCheap: opencastle.c */
static volatile int gApSecrets = 1, gApCandy = 1;   /* secretsanity / candysanity: 0 = those locations stay vanilla */
/* a location left out of the room's pool by its options: no look, no check, the vanilla reward stays */
static int ap_loc_off(int li) { char k = AP_LOCS[li].kind; return (k == 'S' && !gApSecrets) || (k == 'P' && !gApCandy); }
static unsigned char gSrvChecked[AP_NLOCS];       /* the server has it (CHECKED) */
static unsigned char gLocChecked[AP_NLOCS];       /* this save has it (OC_AP_Checked) */
static volatile int gApJob;                       /* the worker asks the game thread to sync */
static unsigned char gLook[AP_NLOCS];             /* what each location holds, as a look code (aplook.h) */
static volatile int gApLookGen;                   /* +1 whenever LOOKS lines arrive */
static int gApActive;                             /* save bound to the connected room */
static u32 gApLevel;                              /* level the save state was read on */
static wchar gSaveBindW[128]; static int gSaveRecv; static wchar gCheckedCsv[4096];
static int gApSentGen = -1, gApItemsApplied = -1;
static int gSaveGoal; static volatile int gApGoalNow, gApDoorOpen;   /* goal reached in this save; worker -> game: just walked in */
/* worker -> game thread: secrets found */
static volatile unsigned short gSecretQ[64]; static volatile int gSecretQN;

static void apshop_text(const char *p); static int apshop_vendor_spawn(u32 obj); static void apshop_tick(void);
static void apshop_worker(u32 level, int settle);   /* apshop.h */
static int ap_loc_index(int off) { int i; for (i = 0; i < AP_NLOCS; i++) if (AP_LOCS[i].off == off) return i; return -1; }
static const struct apitem *ap_item(int off) { int i; for (i = 0; i < AP_NITEMS; i++) if (AP_ITEMS[i].off == off) return &AP_ITEMS[i]; return 0; }
static int a_num(const char **p) { int v = 0, neg = 0; if (**p == '-') { neg = 1; (*p)++; } while (**p >= '0' && **p <= '9') v = v * 10 + (*(*p)++ - '0'); return neg ? -v : v; }
static int a_starts(const char *s, const char *w) { while (*w) if (*s++ != *w++) return 0; return 1; }
static void a_wide(wchar *out, const char *in, int max) { int i; for (i = 0; i < max - 1 && in[i]; i++) out[i] = (wchar)(unsigned char)in[i]; out[i] = 0; }
static void a_narrow(char *out, const wchar *in, int max) { int i; for (i = 0; i < max - 1 && in[i]; i++) out[i] = (char)(in[i] < 128 ? in[i] : '?'); out[i] = 0; }

/* ---- worker: the connection ---------------------------------------------------------------------------------- */
static int gApNetGen = -1;
static void ap_poll(void) {
    static char line[NET_LINE]; static wchar b[300]; int n = 0;
    if (gNetGen != gApNetGen && gNetUp) {
        gApNetGen = gNetGen;
        { char h[64], v[32]; a_narrow(v, OC_VERSION, 32); wsprintfA(h, "HELLO %s", v); net_send(h); }
        gRecvSynced = 0; gApJob = 1;   /* BIND + CHECKS again once the game thread has the save */
    }
    while (n++ < 64 && net_recv_line(line)) {
        const char *p = line;
        if (a_starts(line, "ROOM ")) {
            char *q = line + 5; int f = 0; const char *r;
            /* seed|slot|split|cheap|cards|secrets|candy */
            for (r = q; *r; r++) if (*r == '|' && ++f == 2) break;
            if (f == 2) {
                int k = (int)(r - q); if (k > 127) k = 127;
                memcpy(gApRoom, q, (unsigned)k); gApRoom[k] = 0;
                r++; gApSplit = a_num(&r); if (*r == '|') r++;
                gApCheap = a_num(&r); if (*r == '|') r++;
                gApCards = a_num(&r);
                gApSecrets = gApCandy = 1;
                if (*r == '|') { r++; gApSecrets = a_num(&r); }
                if (*r == '|') { r++; gApCandy = a_num(&r); }
                gApRoomGen++; gApJob = 1;
                { wchar w[128]; a_wide(w, gApRoom, 128);
                  wsprintfW(b, L"ap: room %s (split challenge spells %d, cheap shops %d, cards required %d, secretsanity %d, candysanity %d)",
                            w, gApSplit, gApCheap, gApCards, gApSecrets, gApCandy); }
                log_line(b);
            }
        } else if (a_starts(line, "ITEMS ")) {
            int first; p = line + 6; first = a_num(&p); if (*p == ' ') p++;
            while (*p) { int v = a_num(&p); if (first >= 0 && first < AP_RECVMAX) gRecv[first] = (unsigned short)v; first++; if (*p == ',') p++; else break; }
        } else if (a_starts(line, "SYNCED ")) {
            p = line + 7; n = a_num(&p); if (n > AP_RECVMAX) n = AP_RECVMAX;
            gRecvN = n; gRecvSynced = 1; gApJob = 1;
            wsprintfW(b, L"ap: received items synced (%d)", n); log_line(b);
        } else if (a_starts(line, "ITEM ")) {
            int idx, v; p = line + 5; idx = a_num(&p); if (*p == ' ') p++; v = a_num(&p);
            if (idx >= 0 && idx < AP_RECVMAX && idx <= gRecvN) { gRecv[idx] = (unsigned short)v; if (idx == gRecvN) gRecvN = idx + 1; gApJob = 1; }
            { const struct apitem *it = ap_item(v); wsprintfW(b, L"ap: item %d: %s", idx, it ? it->name : L"(unknown)"); log_line(b); }
        } else if (a_starts(line, "CHECKED ")) {
            p = line + 8;
            while (*p) { int i = ap_loc_index(a_num(&p)); if (i >= 0) gSrvChecked[i] = 1; if (*p == ',') p++; else break; }
        } else if (a_starts(line, "SHOPTEXT ")) {
            apshop_text(line + 9);
        } else if (a_starts(line, "LOOKS ")) {   /* <loc>:<code>,... */
            p = line + 6;
            int got = 0;
            while (*p) { int i = ap_loc_index(a_num(&p)), c = 0; if (*p == ':') { p++; c = a_num(&p); } if (i >= 0) { gLook[i] = (unsigned char)c; got++; } if (*p == ',') p++; else break; }
            gApLookGen++;
            wsprintfW(b, L"ap: looks for %d location(s) received", got); log_line(b);
        } else if (a_starts(line, "MSG ")) {
            wchar w[300]; a_wide(w, line + 4, 280); wsprintfW(b, L"ap: %s", w); log_line(b);
        }
    }
}

/* ---- worker: location actors on this map (secrets, event spawn points, placed pickups and cards) ---- */
static const wchar *const DOXY_CARDS[5] = { L"WC_BrantheBloodthirsty0", L"WC_Cyclops0", L"WC_Goliath0", L"WC_Morholt0", L"WC_HengistofUpperBarnton0" };

static int ap_loc_by_actor(const wchar *map, const wchar *actor, char kind) {   /* kind 0: any */
    int i;
    for (i = 0; i < AP_NLOCS; i++) if (weq(AP_LOCS[i].map, map) && AP_LOCS[i].actor && wieq(AP_LOCS[i].actor, actor) && (!kind || AP_LOCS[i].kind == kind)) return i;
    return -1;
}
#define AP_MAPMAX 128
static u32 gApMapLevel; static int gApMapN; static u32 gApMapObj[AP_MAPMAX]; static int gApMapLoc[AP_MAPMAX];
static float gApMapAt[AP_MAPMAX][3];
static int offFound = -1; static u32 maskFound;
struct apfind { u32 level; const wchar *map; };
static int v_apfind(u32 o, int name, u32 cls, void *c) {
    struct apfind *f = c; int i; wchar nm[64];
    if (!cls || rd32(o + offLevel) != f->level || gApMapN >= AP_MAPMAX || !name_of(name, nm, 64)) return 0;
    for (i = 0; i < AP_NLOCS; i++) {
        const struct aploc *l = &AP_LOCS[i];
        if (!weq(l->map, f->map) || !l->actor || !wieq(l->actor, nm)) continue;
        if (l->kind != 'S' && l->kind != 'E' && l->kind != 'P' && l->kind != 'K') continue;
        if (ap_loc_off(i)) return 0;   /* not in this room's pool: left alone */
        gApMapObj[gApMapN] = o; gApMapLoc[gApMapN] = i;
        rd(o + offLoc, gApMapAt[gApMapN], 12);
        gApMapN++;
        return 0;
    }
    if (weq(f->map, L"HP3_GroundsHub"))   /* the doxy rounds' reward cards */
        for (i = 0; i < 5; i++) if (wieq(nm, DOXY_CARDS[i])) {
            wchar w[16]; int li; wsprintfW(w, L"WinCard%d", i + 1);
            if ((li = ap_loc_by_actor(L"HP3_GroundsHub", w, 'M')) < 0) break;
            gApMapObj[gApMapN] = o; gApMapLoc[gApMapN] = li; rd(o + offLoc, gApMapAt[gApMapN], 12); gApMapN++;
            break;
        }
    return 0;
}
static void ap_map_scan(u32 level, const wchar *map) {   /* worker, once per settled map */
    struct apfind f; static wchar b[120];
    if (offFound < 0) { u32 bp = prop_object(L"SecretAreaMarker", L"bFound"), m = bp ? rd32(bp + 0x70) : 0;
        if (bp && m && !(m & (m - 1)) && rd32(bp + 0x4C) >= 0x28) { offFound = (int)rd32(bp + 0x4C); maskFound = m; } else offFound = 0; }
    gApMapN = 0; f.level = level; f.map = map;
    for_each_object(v_apfind, &f);
    gApMapLevel = level;
    wsprintfW(b, L"ap: %d location actor(s) on this map", gApMapN); log_line(b);
}
static void ap_secrets_poll(void) {   /* worker, every 250 ms on a settled map */
    int i;
    if (offFound <= 0) return;
    for (i = 0; i < gApMapN; i++) {
        int li = gApMapLoc[i];
        if (AP_LOCS[li].kind != 'S' || gLocChecked[li] || !gApMapObj[i]) continue;
        if (rd32(gApMapObj[i] + offFound) & maskFound) {
            if (gSecretQN < 64) gSecretQ[gSecretQN++] = (unsigned short)li;
            gApMapObj[i] = 0;   /* once */
        }
    }
}

/* ---- game thread: the save side -------------------------------------------------------------------------------- */
static void ap_save_write(const wchar *k, const wchar *v) {
    static wchar kk[64], vv[4096]; u32 store = cur_store();
    if (!store) return;
    wcpy(kk, k, 64); wcpy(vv, v, 4096);
    lo_save_value(store, kk, vv);
}
static int ap_save_read(const wchar *k, wchar *v, int max) {
    static wchar kk[64]; u32 store = cur_store();
    v[0] = 0;
    if (!store) return 0;
    wcpy(kk, k, 64);
    return lo_restore_value(store, kk, v, max);
}
static void ap_read_save(void) {
    static wchar v[32]; int i; const wchar *p;
    ap_save_read(L"OC_AP_Bind", gSaveBindW, 128);
    ap_save_read(L"OC_AP_Recv", v, 32);
    gSaveRecv = 0; for (p = v; *p >= '0' && *p <= '9'; p++) gSaveRecv = gSaveRecv * 10 + (*p - '0');
    ap_save_read(L"OC_AP_Checked", gCheckedCsv, 4096);
    ap_save_read(L"OC_AP_Goal", v, 32); gSaveGoal = v[0] == '1';
    for (i = 0; i < AP_NLOCS; i++) gLocChecked[i] = 0;
    for (p = gCheckedCsv; *p; ) {
        int off = 0;
        while (*p >= '0' && *p <= '9') off = off * 10 + (*p++ - '0');
        if ((i = ap_loc_index(off)) >= 0) gLocChecked[i] = 1;
        if (*p == ',') p++; else if (*p) p++;
    }
}
static void ap_send_checks(void) {   /* every location checked in this save, in chunks */
    char line[NET_LINE]; int i, n = 0, len = 0;
    for (i = 0; i < AP_NLOCS; i++) {
        if (!gLocChecked[i]) continue;
        if (!n) { wsprintfA(line, "CHECKS "); len = 7; }
        len += wsprintfA(line + len, n ? ",%d" : "%d", AP_LOCS[i].off); n++;
        if (len > 900) { net_send(line); n = 0; }
    }
    if (n) net_send(line);
}
/* record a check (game thread) and tell the client */
static void ap_check(int li) {
    static wchar b[200]; char line[48];
    if (li < 0 || li >= AP_NLOCS || ap_loc_off(li)) return;
    if (gSaveBindW[0] && !gApActive && gApRoom[0]) return;   /* this save belongs to another room */
    if (!gLocChecked[li]) {
        int len = wlen(gCheckedCsv);
        gLocChecked[li] = 1;
        if (len < 4080) { wsprintfW(gCheckedCsv + len, len ? L",%d" : L"%d", AP_LOCS[li].off); ap_save_write(L"OC_AP_Checked", gCheckedCsv); }
        wsprintfW(b, L"ap: CHECK %s (%d)", AP_LOCS[li].name, AP_LOCS[li].off); log_line(b);
    }
    wsprintfA(line, "CHECK %d", AP_LOCS[li].off);
    net_send(line);
}

/* cards: WizardCardManager.Cards[i].Status (5 = owned). The manager is rebuilt on every map load, so received cards are
   set again each time; the statuses seen at the start of the map are what a vanilla pickup is put back to. */
static int offCardStatus = -1;
static unsigned char gCardStatus0[96]; static u32 gCardStatusLevel;
static u32 ap_cards(int *n) {
    u32 pc = gPlayer ? P(gPlayer + offPlayerActor) : 0, level = pc ? P(pc + offLevel) : 0;
    u32 game = level && offGame > 0 ? P(level + offGame) : 0, wcm;
    *n = 0;
    if (!game || !shop_init()) return 0;
    if (offCardStatus < 0) offCardStatus = struct_member_offset(L"CardDesc", L"Status");
    if (offCardStatus < 0) return 0;
    wcm = P(game + offWCM);
    if (!wcm) return 0;
    *n = (int)P(wcm + offCards + 4);
    return P(wcm + offCards);
}
static int ap_card_received(u32 cls) {   /* is this card class among the received items? */
    int i; wchar nm[64];
    if (!cls || !name_of((int)P(cls + 0x20), nm, 64)) return 0;
    for (i = 0; i < gRecvN; i++) { const struct apitem *it = ap_item(gRecv[i]); if (it && it->kind == 'k' && wieq(it->key, nm)) return 1; }
    return 0;
}
static void ap_cards_apply(void) {
    int n, i; u32 cards = ap_cards(&n), level = cur_level();
    if (!cards || n > 96) return;
    if (level != gCardStatusLevel) { gCardStatusLevel = level; for (i = 0; i < n; i++) gCardStatus0[i] = *(u8 *)(cards + i * CARD_STRIDE + offCardStatus); }
    if (!gApActive) return;
    for (i = 0; i < n; i++) if (ap_card_received(P(cards + i * CARD_STRIDE + offCardCls))) *(u8 *)(cards + i * CARD_STRIDE + offCardStatus) = 5;
}
static void ap_card_undo(u32 cls) {   /* a location's card was picked up the vanilla way: put its status back */
    int n, i; u32 cards = ap_cards(&n);
    if (!cards || n > 96) return;
    for (i = 0; i < n; i++) if (P(cards + i * CARD_STRIDE + offCardCls) == cls) {
        if (!ap_card_received(cls)) *(u8 *)(cards + i * CARD_STRIDE + offCardStatus) = gCardStatus0[i] == 5 ? 0 : gCardStatus0[i];
        return;
    }
}

/* currency and frogs (counted items) */
static int N_SubColl = -1, N_AddColl2 = -1, N_AddHealth = -1; static u32 gClsPastyColl, gClsCakeColl, gClsPastyPU, gClsCakePU, gClsCardPU;
static void ap_currency(int pasties, int cakes, int sub) {
    struct { u32 cls; int n; u32 ret; } parms; u32 carrier = carrier_pawn(); void *fn; int k;
    if (N_SubColl < 0) N_SubColl = name_index(L"SubtractFromInventoryCollection");
    if (N_AddColl2 < 0) N_AddColl2 = name_index(L"AddToInventoryCollection");
    if (!gClsPastyColl) gClsPastyColl = find_class(L"PumpkinPastyCollection");
    if (!gClsCakeColl) gClsCakeColl = find_class(L"CauldronCakeCollection");
    if (!carrier || !(fn = gFindFunction((void *)carrier, sub ? N_SubColl : N_AddColl2, 0))) { log_line(L"ap: inventory functions not found"); return; }
    for (k = 0; k < 2; k++) {
        int amount = k ? cakes : pasties; u32 cls = k ? gClsCakeColl : gClsPastyColl;
        if (amount <= 0 || !cls) continue;
        parms.cls = cls; parms.n = amount; parms.ret = 0;
        gPEOrig((void *)carrier, fn, &parms, 0);
    }
}
static void ap_frog(void) {
    struct { float amount; } parms; u32 pc = gPlayer ? P(gPlayer + offPlayerActor) : 0, pawn = pc ? P(pc + offPawn) : 0; void *fn;
    if (N_AddHealth < 0) N_AddHealth = name_index(L"AddHealthToCompanionsAndSelf");
    if (!pawn || N_AddHealth < 0 || !(fn = gFindFunction((void *)pawn, N_AddHealth, 0))) return;
    parms.amount = 50.0f;   /* ChocolateFrog.HealthToAddToCompanions */
    gPEOrig((void *)pawn, fn, &parms, 0);
}

/* idempotent items: set every flag the received list implies (one store write per missing flag) */
static int ap_flag_missing(const struct apitem *it, const wchar **sec, const wchar **key) {
    int i;
    switch (it->kind) {
    case 's': *sec = L"Spells"; *key = it->key; for (i = 0; i < NSPELLS; i++) if (weq(SPELLS[i].key, it->key)) return !gSaveSpell[i]; return 0;
    case 'd': *sec = L"Doors"; *key = it->key; for (i = 0; i < NDOORS && i < 16; i++) if (weq(DOORS[i].key, it->key)) return !gSaveDoor[i]; return 0;
    case 'w': *sec = L"Items"; *key = it->key; i = it->key[8] - '1'; return i >= 0 && i < 8 && !gSavePortrait[i];
    case 'i': *sec = L"Items"; *key = it->key; return weq(it->key, L"MaraudersMap") ? !gSaveMap : !gSaveShortcuts;
    default: return 0;
    }
}
static void ap_items_apply(void) {
    static wchar b[200], kk[96]; int i, set = 0, from;
    for (i = 0; i < gRecvN; i++) {
        const struct apitem *it = ap_item(gRecv[i]); const wchar *sec = 0, *key = 0; int g;
        if (!it) continue;
        if (ap_flag_missing(it, &sec, &key)) {
            wsprintfW(kk, L"OC_%s_%s", sec, key); ap_save_write(kk, L"1"); set++;
            if (it->kind == 's') { int k; for (k = 0; k < NSPELLS; k++) if (weq(SPELLS[k].key, key)) gSaveSpell[k] = 1; }
        }
        if (it->kind == 'd' && !gApSplit) for (g = 0; g < 2; g++) {   /* a challenge unlock brings its spell(s) */
            const wchar *sp = g ? it->g2 : it->g1; int k;
            if (!sp) continue;
            for (k = 0; k < NSPELLS; k++) if (weq(SPELLS[k].key, sp) && !gSaveSpell[k]) { wsprintfW(kk, L"OC_Spells_%s", sp); ap_save_write(kk, L"1"); gSaveSpell[k] = 1; set++; }
        }
    }
    if (set) { wsprintfW(b, L"ap: %d unlock flag(s) set from received items", set); log_line(b); items_apply(); }
    ap_cards_apply();
    /* counted items, from the save's index on */
    from = gSaveRecv;
    if (from > gRecvN) from = gRecvN;   /* the server knows fewer than the save: a different room was caught above */
    for (i = from; i < gRecvN; i++) {
        const struct apitem *it = ap_item(gRecv[i]);
        if (!it) continue;
        if (it->kind == 'c') ap_currency(it->p, it->c, 0);
        else if (it->kind == 'f') ap_frog();
    }
    if (gRecvN > gSaveRecv) {
        static wchar v[16];
        wsprintfW(b, L"ap: applied received items %d..%d", gSaveRecv, gRecvN - 1); log_line(b);
        gSaveRecv = gRecvN; wsprintfW(v, L"%d", gSaveRecv); ap_save_write(L"OC_AP_Recv", v);
    }
}

/* game thread, safe tick: the sync job */
static int gApMismatchSent;
static void ap_game(void) {
    static wchar b[300]; static char line[200]; u32 level = cur_level(); wchar room[128];
    if (!level || !cur_store()) return;
    while (gSecretQN > 0) ap_check(gSecretQ[--gSecretQN]);
    if (!gApJob) return;
    gApJob = 0;
    if (level != gApLevel) { gApLevel = level; ap_read_save(); gApMismatchSent = 0; gApSentGen = -1; }
    a_wide(room, gApRoom, 128);
    if (gNetUp && gApSentGen != gApNetGen) {
        char bind[140]; a_narrow(bind, gSaveBindW, 128);
        wsprintfA(line, "BIND %s", bind[0] ? bind : "-"); net_send(line);
    }
    gApActive = 0;
    if (gApRoom[0]) {
        if (!gSaveBindW[0]) {   /* a save not bound yet (New Game, or one from before Archipelago): bind it now */
            wcpy(gSaveBindW, room, 128); ap_save_write(L"OC_AP_Bind", gSaveBindW);
            wsprintfW(b, L"ap: this save is now bound to %s", room); log_line(b);
            if (gNetUp) { wsprintfA(line, "BIND %s", gApRoom); net_send(line); }
        }
        if (weq(gSaveBindW, room)) gApActive = 1;
        else if (!gApMismatchSent) { gApMismatchSent = 1; net_send("MISMATCH");
            wsprintfW(b, L"ap: this save belongs to %s, the client is on %s: nothing applied or sent", gSaveBindW, room); log_line(b); }
    }
    if (gApActive && gNetUp && gApSentGen != gApNetGen) { gApSentGen = gApNetGen; ap_send_checks(); if (gSaveGoal) net_send("GOAL"); }
    if (gApActive && gRecvSynced) ap_items_apply();
    else ap_cards_apply();   /* keeps the start-of-map statuses for the undo */
}

/* ---- game thread: pickups and containers ---------------------------------------------------------------------- */
#define AP_PUMAX 64
static u32 gApPu[AP_PUMAX]; static int gApPuLoc[AP_PUMAX]; static int gApNPu;   /* spawned pickups tied to a location */
static volatile u32 gApKill[32]; static volatile int gApNKill;                   /* goodies to destroy on the next tick */
static int ap_is_goodie(u32 obj) {
    if (!gClsPastyPU) gClsPastyPU = find_class(L"PumpkinPasty");
    if (!gClsCakePU) gClsCakePU = find_class(L"CauldronCake");
    if (!gClsCardPU) gClsCardPU = find_class(L"WizardCard");
    return (gClsPastyPU && is_a_fast(obj, gClsPastyPU)) ? 1 : (gClsCakePU && is_a_fast(obj, gClsCakePU)) ? 2 : (gClsCardPU && is_a_fast(obj, gClsCardPU)) ? 3 : 0;
}
static int ap_tied(int li) { int i; for (i = 0; i < gApNPu; i++) if (gApPu[i] && gApPuLoc[i] == li) return 1; return 0; }
static void ap_look_queue(u32 obj, int li); static void ap_look_forget(u32 obj); static void ap_looks_tick(void);   /* aplook.h */
/* PostBeginPlay of a KWPickup, after leftovers.h has (maybe) linked it to a spawner */
static void ap_pickup_spawned(u32 obj, int linkedBefore) {
    wchar nm[64]; const wchar *map = (const wchar *)gCurMap; int li, i;
    if (!map || P(obj + offLevel) != cur_level()) return;
    if (apshop_vendor_spawn(obj)) return;   /* the card a student vendor throws after a sale */
    if (gNPu > linkedBefore && gPu[gNPu - 1].obj == obj) {   /* a container's goodie */
        struct lo_sp *s = &gSp[gPu[gNPu - 1].sp];
        if (!name_of(s->name, nm, 64) || (li = ap_loc_by_actor(map, nm, 'C')) < 0 || !ap_is_goodie(obj)) return;
        /* the container's first pasty / cake / card stands for the location's item (shown as it, checked when picked
           up); every other one is destroyed, the item being the Archipelago one. Already checked: all destroyed. */
        if (!gLocChecked[li] && !ap_tied(li) && gApNPu < AP_PUMAX) {
            gApPu[gApNPu] = obj; gApPuLoc[gApNPu] = li; gApNPu++;
            ap_look_queue(obj, li);
        } else if (gApNKill < 32) {
            gApKill[gApNKill++] = obj;
            gPu[gNPu - 1].state = 1; gPu[gNPu - 1].obj = 0;   /* leftovers.h: collected, never thrown out again */
        }
        return;
    }
    /* an event spawn: the nearest event location on this map, within 150 units */
    {   float *at = (float *)(obj + offLoc), bd = 150.0f * 150.0f; int best = -1;
        for (i = 0; i < gApMapN; i++) {
            float dx, dy, dz, d2; const struct aploc *l = &AP_LOCS[gApMapLoc[i]];
            if (l->kind != 'E' && !(l->kind == 'K' && !l->key[0])) continue;   /* spawn points only */
            dx = at[0] - gApMapAt[i][0]; dy = at[1] - gApMapAt[i][1]; dz = at[2] - gApMapAt[i][2]; d2 = dx * dx + dy * dy + dz * dz;
            if (d2 < bd) { bd = d2; best = gApMapLoc[i]; }
        }
        if (best >= 0 && gLocChecked[best] && gApActive && !ap_loc_off(best) && gApNKill < 32) gApKill[gApNKill++] = obj;   /* checked already */
        else if (best >= 0 && gApNPu < AP_PUMAX) { gApPu[gApNPu] = obj; gApPuLoc[gApNPu] = best; gApNPu++; ap_look_queue(obj, best); }
    }
}
/* Destroyed of a KWPickup: picked up? */
static int gApUndoP, gApUndoC; static u32 gApUndoCard;
/* pickups apshop.h's Touch took over: the vanilla reward was never given, so there is nothing to take back */
static u32 gApTaken[16]; static int gApTakenN;
static int ap_taken(u32 obj) { int i; for (i = 0; i < 16; i++) if (gApTaken[i] == obj) { gApTaken[i] = 0; return 1; } return 0; }
static void ap_pickup_destroyed(u32 obj) {
    wchar nm[64]; const wchar *map = (const wchar *)gCurMap; int li = -1, i, what;
    ap_look_forget(obj);
    for (i = 0; i < gApMapN; i++) if (gApMapObj[i] == obj && AP_LOCS[gApMapLoc[i]].kind != 'S') gApMapObj[i] = 0;
    if (!map || offPicked <= 0 || P(obj + offPicked) != 1) {
        for (i = 0; i < gApNPu; i++) if (gApPu[i] == obj) gApPu[i] = 0;
        return;
    }
    for (i = 0; i < gApNPu; i++) if (gApPu[i] == obj) { li = gApPuLoc[i]; gApPu[i] = 0; break; }
    if (li < 0 && name_of((int)P(obj + 0x20), nm, 64)) {
        li = ap_loc_by_actor(map, nm, 0);
        if (li >= 0 && AP_LOCS[li].kind != 'P' && AP_LOCS[li].kind != 'K') li = -1;
        if (li < 0 && weq(map, L"HP3_GroundsHub"))
            for (i = 0; i < 5; i++) if (wieq(nm, DOXY_CARDS[i])) { wchar w[16]; wsprintfW(w, L"WinCard%d", i + 1); li = ap_loc_by_actor(L"HP3_GroundsHub", w, 'M'); }
    }
    if (li < 0 || ap_loc_off(li)) return;   /* not a location, or one this room leaves vanilla: keep the reward */
    ap_check(li);
    if (ap_taken(obj)) return;              /* picked up through apshop.h's Touch: nothing was given */
    what = ap_is_goodie(obj);   /* take the vanilla reward back on the next tick */
    if (what == 1) gApUndoP++; else if (what == 2) gApUndoC++; else if (what == 3) gApUndoCard = P(obj + 0x24);
}
/* ---- the goal: the Final Day door --------------------------------------------------------------------------- */
static u32 gFDDoor, gFDLevel, gFDLast;
static int ap_cards_received(void) {
    int i, n = 0;
    for (i = 0; i < gRecvN; i++) { const struct apitem *it = ap_item(gRecv[i]); if (it && it->kind == 'k') n++; }
    return n;
}
static int ap_goal_ready(void) { return gApActive && gRecvSynced && gApCards > 0 && ap_cards_received() >= gApCards; }
struct fdfind { u32 level; int name; u32 found; };
static int v_fdfind(u32 o, int name, u32 cls, void *c) {
    struct fdfind *f = c;
    if (name != f->name || !cls || rd32(o + offLevel) != f->level || !is_a_cls(cls, gClsMover)) return 0;
    f->found = o; return 1;
}
/* game thread, every tick on the grounds: open the door once the cards are in (and keep it open) */
static void ap_final_door(void) {
    static wchar b[160]; u32 level = cur_level(); struct { u32 other, instigator; } parms; void *fn;
    if (!weq((const wchar *)gCurMap, L"HP3_GroundsHub") || !ap_goal_ready()) { if (level != gFDLevel) gApDoorOpen = 0; return; }
    if (level != gFDLevel) {
        struct fdfind f; f.level = level; f.name = name_index(L"Mover11"); f.found = 0;
        gFDLevel = level; gFDDoor = 0; gFDLast = 0;
        if (f.name >= 0 && gClsMover) for_each_object(v_fdfind, &f);
        gFDDoor = f.found;
        wsprintfW(b, L"ap: Final Day door (Mover11) %s", gFDDoor ? L"found" : L"NOT FOUND"); log_line(b);
    }
    if (!gFDDoor || offKeyNum < 0) return;
    if ((P(gFDDoor + offKeyNum) & 0xFF) != 0) {   /* open (or opening): InterpolateTo sets KeyNum to the target at once */
        if (!gApDoorOpen) { gApDoorOpen = 1; net_send("DOOR"); log_line(L"ap: the Final Day door is open"); }
        return;
    }
    if (gFDLast && GetTickCount() - gFDLast < 3000) return;
    gFDLast = GetTickCount();
    if (N_Trigger < 0 || !(fn = gFindFunction((void *)gFDDoor, N_Trigger, 0))) return;
    parms.other = parms.instigator = 0;
    gPEOrig((void *)gFDDoor, fn, &parms, 0);   /* as vanilla's "Trigger FinalDayDoor" */
    log_line(L"ap: opening the Final Day door");
}
/* game thread: the player walked in */
static void ap_goal(void) {
    gSaveGoal = 1; ap_save_write(L"OC_AP_Goal", L"1");
    net_send("GOAL");
    log_line(L"ap: GOAL: through the Final Day door");
}

/* game thread, safe tick */
static void ap_tick(void) {
    int i;
    for (i = 0; i < gApNKill; i++) { u32 a = gApKill[i], xl = a && offXLevel > 0 ? P(a + offXLevel) : 0; if (xl && gDestroyActor) gDestroyActor((void *)xl, (void *)a, 0); }
    gApNKill = 0;
    if (gApUndoP || gApUndoC) { ap_currency(gApUndoP, gApUndoC, 1); gApUndoP = gApUndoC = 0; }
    if (gApUndoCard) { ap_card_undo(gApUndoCard); gApUndoCard = 0; }
    ap_game();
    apshop_tick();
    ap_looks_tick();
    ap_final_door();
    if (gApGoalNow) { gApGoalNow = 0; ap_goal(); }
}
/* from pe_hook, before the event runs (Destroyed) or after leftovers.h (PostBeginPlay) */
static void ap_event(u32 self, int fname, int linkedBefore) {
    if (!gClsPickup || !is_a_fast(self, gClsPickup)) return;
    if (fname == N_PostBeginPlay) ap_pickup_spawned(self, linkedBefore);
    else if (fname == N_Destroyed) ap_pickup_destroyed(self);
}
/* worker, every 250 ms */
static void ap_worker(u32 level, const wchar *map, int settle) {
    ap_poll();
    if (settle == 12 || (settle > 12 && level != gApMapLevel)) { ap_map_scan(level, map); gApNPu = 0; gApJob = 1; }
    if (settle > 12) ap_secrets_poll();
    apshop_worker(level, settle);
    /* past the open Final Day door, where vanilla's Final Day cutscene triggers stand: the goal, then HP3_Final */
    if (settle > 12 && gApDoorOpen && weq(map, L"HP3_GroundsHub") && !gPending) {
        static int armed; u32 pc = gPlayer ? rd32(gPlayer + offPlayerActor) : 0, pawn = pc ? rd32(pc + offPawn) : 0; float at[3];
        if (pawn && rd(pawn + offLoc, at, 12)) {
            int in = at[0] > 860.0f && at[0] < 1250.0f && at[1] > -6330.0f && at[1] < -5990.0f && at[2] > 820.0f && at[2] < 1120.0f;
            if (!in) armed = 1;
            else if (armed) {
                armed = 0; gApGoalNow = 1;
                wcpy(gPendingURL, L"HP3_Final", 128); gPending = 1;   /* travels on the next tick (do_travel) */
                log_line(L"ap: walked through the Final Day door -> HP3_Final");
            }
        }
    }
}
