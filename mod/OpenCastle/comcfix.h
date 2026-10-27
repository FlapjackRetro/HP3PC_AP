/*
 * COMC (hippogriff) fix: track 1 must give Joscelind Wadcock.
 *
 * HP3_COMC has two HippoFlyGame managers. The tutorial one (HippoFlyGame1, group gstate040) plays track 1 on the
 * first visit; its win runs 4076COMCExit, which spawns WizCard1 = Joscelind Wadcock. The course-selection one
 * (HippoFlyGame2, every state but 040) has CardWinEvent[0] = COMCWin_WizCard2QC, the same as track 2's, a
 * copy-paste slip that vanilla never shows (track 1 is always won in 040 first). Open Castle runs COMC in
 * GState090, so track 1 there gave Gwenog Jones (seen in testing).
 *
 * Fix: while the player is on HP3_COMC, keep the WizCard2 spawner's (HPSpawnThingy2) SpawnClass in step with the
 * track being played: track 1 (index 0) -> WC_JoscelindWadcock, otherwise WC_GwenogJones. The card is spawned by
 * the win cutscene a few seconds after the race, long after CurGame.Track is set at the start of the race.
 */
static int offCurGame = -1, offTrack = -1, offSpawnClass = -1;
static u32 gClsHippo, gClsSpawnThingy, gClsWCJoscelind, gClsWCGwenog;
struct comcfind { u32 level; int tagGame, tagCard; u32 game, card; };
static int v_comc(u32 o, int name, u32 cls, void *c) {
    struct comcfind *f = c; int tag; (void)name;
    if (!cls || rd32(o + offLevel) != f->level) return 0;
    tag = (int)rd32(o + offTag);
    if (tag == f->tagGame && is_a_cls(cls, gClsHippo)) f->game = o;
    if (tag == f->tagCard && is_a_cls(cls, gClsSpawnThingy)) f->card = o;
    return 0;
}
static void comc_poll(u32 level, const wchar *map) {
    static u32 lvl, game, card; static int lastTrack = -2; static wchar b[160]; int track; u32 want;
    if (!weq(map, L"HP3_COMC")) { lvl = 0; return; }
    if (offCurGame < 0) {
        offCurGame = prop_offset(L"HippoFlyGame", L"CurGame"); offTrack = struct_member_offset(L"CurGameStruct", L"Track");
        offSpawnClass = prop_offset(L"SpawnThingy", L"SpawnClass");
        gClsHippo = find_class(L"HippoFlyGame"); gClsSpawnThingy = find_class(L"SpawnThingy");
        gClsWCJoscelind = find_class(L"WC_JoscelindWadcock"); gClsWCGwenog = find_class(L"WC_GwenogJones");
        wsprintfW(b, L"comc: offsets curgame=%d track=%d spawnclass=%d; classes %s", offCurGame, offTrack, offSpawnClass,
                  gClsHippo && gClsSpawnThingy && gClsWCJoscelind && gClsWCGwenog ? L"ok" : L"MISSING");
        log_line(b);
    }
    if (offCurGame <= 0 || offTrack < 0 || offSpawnClass <= 0 || offTag <= 0 || !gClsWCJoscelind || !gClsWCGwenog) return;
    if (lvl != level) {
        struct comcfind f; f.level = level; f.tagGame = name_index(L"HippoFlyGame"); f.tagCard = name_index(L"WizCard2"); f.game = f.card = 0;
        for_each_object(v_comc, &f);
        lvl = level; game = f.game; card = f.card; lastTrack = -2;
        wsprintfW(b, L"comc: course manager %s, WizCard2 spawner %s", game ? L"found" : L"NOT FOUND", card ? L"found" : L"NOT FOUND"); log_line(b);
    }
    if (!game || !card) return;
    track = (int)rd32(game + offCurGame + offTrack);
    want = track == 0 ? gClsWCJoscelind : gClsWCGwenog;
    if (rd32(card + offSpawnClass) != want) *(volatile u32 *)(card + offSpawnClass) = want;
    if (track != lastTrack) {
        lastTrack = track;
        wsprintfW(b, L"comc: track %d selected; its card spawner gives %s", track + 1, track == 0 ? L"Joscelind Wadcock" : L"Gwenog Jones (track 2's)");
        if (track >= 0 && track < 5) log_line(b);
    }
}
