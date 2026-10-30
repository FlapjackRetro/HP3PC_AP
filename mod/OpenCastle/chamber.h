/*
 * The challenge-chamber copies at the end of each spell challenge (B maps). Each of HP3_Ch1CarpeRetractumB,
 * HP3_Ch2DraconiforsB and HP3_Ch3GlaciusB ends in its own copy of InsideHub's challenge chamber: three portcullises
 * (KWMover, bPersistent) in front of ChangeLevelTriggers straight into HP3_Ch1CarpeRetractum, HP3_Ch2Draconifors and
 * HP3_Ch3Glacius. Vanilla filters most of these portcullises out of GState090 (ExcludeFromGameStates: hidden,
 * collision off, via Actor.FilterForCurrentGameState, which only runs at level start), so the passages stand open;
 * Glacius B's own Glacius portcullis is there and shut.
 *
 * Open Castle makes them match InsideHub: on these maps each chamber portcullis is shown with collision on and kept
 * at the key that matches its challenge (raised = unlocked, lowered = locked), and that door's ChangeLevelTrigger has
 * its collision off while the challenge is locked. The travel hook still refuses a locked challenge either way.
 *
 * Down / up keys differ per copy (KeyPos(1) from the maps):
 *   Carpe B, Draconifors B: key 0 = down, key 1 = up (+160..+208)   -> inv 0 (open = key 1)
 *   Glacius B: the Carpe and Draconifors ports key 0 = up, key 1 = down (-136) -> inv 1; its Glacius port key 0 =
 *   down (+184) -> inv 0
 */
struct chamber { const wchar *map, *mover, *trig, *key; int inv; };
static const struct chamber CHAMBERS[] = {
    { L"HP3_Ch1CarpeRetractumB", L"KWMover31", L"ChangeLevelTrigger9",  L"ChCarpe",   0 },   /* ChalChamb_CR_PortCullis */
    { L"HP3_Ch1CarpeRetractumB", L"KWMover33", L"ChangeLevelTrigger10", L"ChDracLap", 0 },   /* ChalChamb_Drac_Port */
    { L"HP3_Ch1CarpeRetractumB", L"KWMover32", L"ChangeLevelTrigger11", L"ChGlacius", 0 },   /* ChalChamb_Gla_Port */
    { L"HP3_Ch2DraconiforsB",    L"KWMover31", L"ChangeLevelTrigger0",  L"ChCarpe",   0 },   /* ChalChamb_CR_Port */
    { L"HP3_Ch2DraconiforsB",    L"KWMover32", L"ChangeLevelTrigger2",  L"ChDracLap", 0 },   /* ChalChamb_Drac_Port */
    { L"HP3_Ch2DraconiforsB",    L"KWMover33", L"ChangeLevelTrigger3",  L"ChGlacius", 0 },   /* ChalChamb_Gla_Port */
    { L"HP3_Ch3GlaciusB",        L"KWMover31", L"ChangeLevelTrigger3",  L"ChCarpe",   1 },   /* ChalChamb_CR_Port */
    { L"HP3_Ch3GlaciusB",        L"KWMover32", L"ChangeLevelTrigger4",  L"ChDracLap", 1 },   /* ChalChamb_Drac_Port */
    { L"HP3_Ch3GlaciusB",        L"KWMover33", L"ChangeLevelTrigger0",  L"ChGlacius", 0 },   /* ChalChamb_Gla_Port */
};
#define NCHAMBERS (int)(sizeof CHAMBERS / sizeof CHAMBERS[0])

static u32 gChLevel, gChMover[NCHAMBERS], gChTrig[NCHAMBERS], gChLast[NCHAMBERS];
static int gChTrigOff[NCHAMBERS];                       /* we switched this trigger's collision off */
static volatile u32 gChShow, gChFire, gChTrigSet, gChTrigOn;   /* game-thread jobs, bit per CHAMBERS row */
static int offChHidden = -1, offChCollide = -1; static u32 maskChHidden, maskChCollide;

struct chfind { u32 level; int mover[NCHAMBERS], trig[NCHAMBERS]; };
static int v_chamber(u32 o, int name, u32 cls, void *c) {
    struct chfind *f = c; int i;
    if (!cls || rd32(o + offLevel) != f->level) return 0;
    for (i = 0; i < NCHAMBERS; i++) {
        if (f->mover[i] >= 0 && name == f->mover[i] && is_a_cls(cls, gClsMover)) gChMover[i] = o;
        if (f->trig[i] >= 0 && name == f->trig[i]) gChTrig[i] = o;
    }
    return 0;
}
static int ch_bool(const wchar *prop, int *off, u32 *mask) {   /* UBoolProperty: offset +0x4C, BitMask +0x70 */
    u32 bp, m;
    if (*off > 0) return 1;
    if (!(bp = prop_object(L"Actor", prop))) return 0;
    m = rd32(bp + 0x70);
    if (!m || (m & (m - 1)) || rd32(bp + 0x4C) < 0x28) return 0;
    *off = (int)rd32(bp + 0x4C); *mask = m;
    return 1;
}

/* worker, from gates_poll (every 2 s), after gDoorOn is up to date */
static void chamber_poll(u32 level, const wchar *map) {
    static wchar b[200]; int i, any = 0;
    for (i = 0; i < NCHAMBERS; i++) if (weq(CHAMBERS[i].map, map)) any = 1;
    if (!any) { gChLevel = 0; return; }
    if (!ch_bool(L"bHidden", &offChHidden, &maskChHidden) || !ch_bool(L"bCollideActors", &offChCollide, &maskChCollide)
        || !gSetCollision) return;
    if (level != gChLevel) {
        struct chfind f;
        f.level = level;
        for (i = 0; i < NCHAMBERS; i++) {
            gChMover[i] = gChTrig[i] = 0; gChLast[i] = 0; gChTrigOff[i] = 0;
            f.mover[i] = weq(CHAMBERS[i].map, map) ? name_index(CHAMBERS[i].mover) : -1;
            f.trig[i]  = weq(CHAMBERS[i].map, map) ? name_index(CHAMBERS[i].trig) : -1;
        }
        for_each_object(v_chamber, &f);
        gChLevel = level;
        for (i = 0; i < NCHAMBERS; i++) if (weq(CHAMBERS[i].map, map)) {
            wsprintfW(b, L"chamber %s: portcullis %s %s, loading trigger %s %s", CHAMBERS[i].key, CHAMBERS[i].mover,
                      gChMover[i] ? L"found" : L"NOT FOUND", CHAMBERS[i].trig, gChTrig[i] ? L"found" : L"NOT FOUND");
            log_line(b);
        }
    }
    for (i = 0; i < NCHAMBERS; i++) {
        int on = 0, d, open; u32 m = gChMover[i], t = gChTrig[i];
        if (!weq(CHAMBERS[i].map, map)) continue;
        for (d = 0; d < NDOORS; d++) if (weq(DOORS[d].key, CHAMBERS[i].key)) { on = gDoorOn[d]; break; }
        if (m) {
            if ((rd32(m + offChHidden) & maskChHidden) || !(rd32(m + offChCollide) & maskChCollide)) {
                if (!((gChShow >> i) & 1)) { wsprintfW(b, L"chamber %s: portcullis shown (vanilla hides it in this game state)", CHAMBERS[i].key); log_line(b); }
                gChShow |= 1u << i;
            }
            open = (rd32(m + offKeyNum) & 0xFF) != 0;   /* InterpolateTo sets KeyNum to the target key at once */
            if (CHAMBERS[i].inv) open = !open;
            if (open != on && !(gChLast[i] && GetTickCount() - gChLast[i] < 3000)) {
                gChLast[i] = GetTickCount(); gChFire |= 1u << i;
                wsprintfW(b, L"chamber %s: portcullis %s", CHAMBERS[i].key, on ? L"raising" : L"lowering"); log_line(b);
            }
        }
        if (t) {
            int col = (rd32(t + offChCollide) & maskChCollide) != 0;
            if (!on && col) {
                gChTrigOff[i] = 1; gChTrigOn &= ~(1u << i); gChTrigSet |= 1u << i;
                wsprintfW(b, L"chamber %s: loading trigger off (locked)", CHAMBERS[i].key); log_line(b);
            } else if (on && gChTrigOff[i] && !col) {
                gChTrigOff[i] = 0; gChTrigOn |= 1u << i; gChTrigSet |= 1u << i;
                wsprintfW(b, L"chamber %s: loading trigger on (unlocked)", CHAMBERS[i].key); log_line(b);
            }
        }
    }
}

/* game thread (pe_hook, tick events only) */
static void chamber_apply(void) {
    struct { u32 other, instigator; } parms; void *fn; int i;
    u32 show = gChShow, fire = gChFire, set = gChTrigSet, ton = gChTrigOn;
    gChShow = gChFire = gChTrigSet = 0;
    for (i = 0; i < NCHAMBERS; i++) {
        u32 m = gChMover[i], t = gChTrig[i];
        if (((show >> i) & 1) && m && gSetCollision) {
            *(volatile u32 *)(m + offChHidden) &= ~maskChHidden;
            gSetCollision((void *)m, 1, 1, 1);
        }
        if (((fire >> i) & 1) && m && N_Trigger >= 0 && (fn = gFindFunction((void *)m, N_Trigger, 0)) != 0) {
            parms.other = parms.instigator = 0;
            gPEOrig((void *)m, fn, &parms, 0);   /* state-aware: TriggerToggle.Trigger */
        }
        if (((set >> i) & 1) && t && gSetCollision) gSetCollision((void *)t, ((ton >> i) & 1) ? 1 : 0, 0, 0);
    }
}
