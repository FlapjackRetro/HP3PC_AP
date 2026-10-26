/*
 * Open Castle travel rules: rewrite a travel URL (the text given to FURL::FURL) for Open Castle.
 * Pure functions (no Windows calls), so they can be tested on the host (test/rules_test.c).
 * Needs: wchar, wieq, wcpy, wlen, known_map (ue2read.h).
 *
 * URL shape: [path\]Map[.unr][?opt[?opt...]][#portal]
 */
#define OC_STATE L"GState090"

static int starts_ci(const wchar *s, const wchar *p) { while (*p) { if (wlow(*s) != wlow(*p)) return 0; s++; p++; } return 1; }
static int contains_ci(const wchar *s, const wchar *p) { for (; *s; s++) if (starts_ci(s, p)) return 1; return 0; }

static int is_map(const wchar *a, const wchar *b) { return a && b && wieq(a, b); }

/* maps Open Castle never sends the player to (cutscene only, no checks) */
static int dropped_map(const wchar *m) {
    static const wchar *D[] = { L"HP3_Quidditch", L"HP3_WhompingWillow", L"HP3_BuckyExecuted", L"HP3_DementorBattle",
                                L"HP3_Infirmary", L"HP3_DarkTower", L"HP3_GroundsDada" };
    unsigned i;
    for (i = 0; i < sizeof D / sizeof D[0]; i++) if (is_map(m, D[i])) return 1;
    return 0;
}

/* Decide the new destination. src = map the player is leaving ("" if unknown), dst = canonical destination,
   had_unr = the URL named the map with ".unr" (InsideHub's side-room doors do; the classroom triggers don't).
   Returns the new map (may equal dst) and a short reason in *why (0 if unchanged). */
static const wchar *oc_route(const wchar *src, const wchar *dst, int had_unr, const wchar **why) {
    *why = 0;
    if (is_map(dst, L"HP3_Adv1Express") && (!src[0] || is_map(src, L"HP_Preamble") || is_map(src, L"Entry")))
        { *why = L"new game starts in Hogwarts"; return L"HP3_InsideHub"; }
    if (is_map(src, L"HP3_InsideHub") && !had_unr) {   /* classroom triggers: challenges -> final exams */
        if (is_map(dst, L"HP3_Ch1CarpeRetractum")) { *why = L"DADA classroom -> Ron's exam"; return L"HP3_FA1Ron"; }
        if (is_map(dst, L"HP3_Ch2Draconifors"))    { *why = L"Transfiguration -> Hermione's exam"; return L"HP3_FA2Hermione"; }
        if (is_map(dst, L"HP3_Ch3Glacius"))        { *why = L"Charms -> Harry's exam"; return L"HP3_FA3Harry"; }
    }
    if (dropped_map(dst)) {
        if (is_map(src, L"HP3_Adv5Shack")) { *why = L"Shack exit -> dungeon door"; return L"HP3_DungeonHUB"; }
        *why = L"dropped story map -> InsideHub"; return L"HP3_InsideHub";
    }
    if (is_map(src, L"HP3_Adv2ExpPatronum") && is_map(dst, L"HP3_Adv3Library")) { *why = L"Expecto exit -> DADA"; return L"HP3_InsideHub"; }
    if (is_map(src, L"HP3_FA1Ron") && is_map(dst, L"HP3_FA2Hermione"))           { *why = L"exam chain broken"; return L"HP3_InsideHub"; }
    if (is_map(src, L"HP3_FA2Hermione") && is_map(dst, L"HP3_FA3Harry"))         { *why = L"exam chain broken"; return L"HP3_InsideHub"; }
    if (is_map(src, L"HP3_FA3Harry") && is_map(dst, L"HP3_GroundsHub"))          { *why = L"exam chain broken"; return L"HP3_InsideHub"; }
    return dst;
}

/* Rewrite `in` into `out`. Returns 1 if the URL changed. `why` gets a short reason. Unknown maps (menus, save
   packages) and URLs that load or save a game pass through untouched. */
static int oc_rewrite(const wchar *src, const wchar *in, wchar *out, int outmax, wchar *why, int whymax) {
    wchar tok[128], opts[512]; const wchar *p, *map_start, *q, *portal = 0, *dst, *ndst, *reason;
    int n = 0, had_unr = 0, o = 0, keep, changed_state = 0;
    why[0] = 0;
    if (!in || !in[0]) return 0;
    /* map token: after the last path separator, before '?' or '#' */
    map_start = in;
    for (p = in; *p && *p != '?' && *p != '#'; p++) if (*p == '\\' || *p == '/') map_start = p + 1;
    for (p = map_start; *p && *p != '?' && *p != '#' && n < 127; p++) tok[n++] = *p;
    tok[n] = 0;
    if (n > 4 && wieq(tok + n - 4, L".unr")) { had_unr = 1; tok[n - 4] = 0; }
    dst = known_map(tok);
    if (!dst) return 0;
    for (q = p; *q; q++) if (*q == '#') { portal = q; break; }
    if (contains_ci(p, L"save") || contains_ci(p, L"load")) return 0;
    ndst = oc_route(src ? src : L"", dst, had_unr, &reason);
    /* options without any GameState=..., then our state */
    opts[0] = 0;
    while (*p == '?') {
        const wchar *s = ++p;
        while (*p && *p != '?' && *p != '#') p++;
        keep = !starts_ci(s, L"GameState=");
        if (!keep && !(p - s == 19 && starts_ci(s, L"GameState=" OC_STATE))) changed_state = 1;
        if (keep && o + (p - s) + 2 < 500) { opts[o++] = '?'; while (s < p) opts[o++] = *s++; }
    }
    opts[o] = 0;
    if (!is_map(ndst, L"HP_Preamble") && !is_map(ndst, L"Entry")) {
        const wchar *add = L"?GameState=" OC_STATE; int had = contains_ci(in, L"GameState=");
        if (!had) changed_state = 1;
        while (*add && o < 510) opts[o++] = *add++;
        opts[o] = 0;
    }
    if (ndst == dst && !changed_state) return 0;
    /* compose: original path prefix kept only if the map didn't change */
    n = 0;
    if (ndst == dst) { for (q = in; q < map_start && n < outmax - 1; q++) out[n++] = *q; }
    for (q = ndst; *q && n < outmax - 1; q++) out[n++] = *q;
    if (ndst == dst && had_unr) for (q = L".unr"; *q && n < outmax - 1; q++) out[n++] = *q;
    for (q = opts; *q && n < outmax - 1; q++) out[n++] = *q;
    if (portal) for (q = portal; *q && n < outmax - 1; q++) out[n++] = *q;
    out[n] = 0;
    if (reason) wcpy(why, reason, whymax); else wcpy(why, L"story state forced to " OC_STATE, whymax);
    return 1;
}
