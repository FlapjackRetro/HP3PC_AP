/*
 * Shopsanity: Fred & George's shop and the student vendors as Archipelago checks (HGame, decompiled).
 *
 * The sale machinery (one VendorManager, one dialog for everyone: HGame.HPPageVendor):
 *   stands (InsideHub)    VendorItem actors: 6 VI_WizardCard (the twins' cards), 3 VI_PortraitPassword, plus pasty /
 *                         cake / frog stands that stay vanilla. Bump -> state DoingVendorTransaction -> VendorManager.
 *                         InitiateTransaction(None, stand) -> stand.PrepareForSale, price check -> HPPageVendor -> on
 *                         "Buy" stand.Purchased() (cards: WizardCardInvItem.GiveTo; passwords: AddPassword(next
 *                         unowned)), which hides the stand and enters state PostTransaction; NotPurchased (declined,
 *                         can't afford, manager busy) enters PostTransaction with the stand still shown. Timer -> Reload
 *                         re-fills the stand (next shop card / password) 5 s later.
 *   student vendors       HPGenericStudent, state DoingVendorTransaction: spawns a VI_StudentWizardCard for
 *                         WizardCardManager.GetAvailableVendorCard() (the first card with Status 1), price from the card
 *                         table, InitiateTransaction(student, item); on "Buy" the student throws that card's pickup at
 *                         Harry (WizardCard, state ThrownPickup) and picking it up owns the card.
 *
 * What the mod does (only while the save is bound to the connected room):
 *   twins' cards          each of the 6 stands is given one unchecked "F&G Shop: <card>" location (WizardCardClass, cost,
 *                         currency from that card's table row), shows that location's item (aplook.h), and is hidden
 *                         once there are fewer unchecked ones than stands. Their Timer (Reload) is skipped: the mod
 *                         refills them. A purchase (PostTransaction with the stand hidden) checks the location; the card
 *                         Purchased gave is taken back like any vanilla card reward.
 *   passwords             a stand sells "the next unowned password", and the table's Owned flags are the received
 *                         passwords (portraits open with them). From the stand's DoingVendorTransaction until its
 *                         PostTransaction the flags are swapped for the checked password locations, so the next one is
 *                         the first unchecked location (its price, its number for AddPassword); then the received ones
 *                         are put back. All three stands show that location's item; hidden once all 8 are checked.
 *   student vendors       exactly one card has Status 1 while a purchase is unchecked (purchase k's own card unless it was
 *                         received, then another of the 20 that wasn't); every other Status-1 card becomes 5 (received)
 *                         or 6 (OwnedByVendor). During the student's DoingVendorTransaction that card's table price is
 *                         purchase k's vanilla price (cheap: halved), so logic's order holds whichever card is on sale.
 *                         The thrown card is tied to purchase k: it shows k's item, and picking it up checks k (the card
 *                         status goes back as for any card location).
 *   dialog                HPPageVendor's picture and its two text lines (name, description) show the location's item:
 *                         the client sends SHOPTEXT <loc>|<name>|<description> for the shop / vendor locations; the
 *                         picture comes from the look code. Patched on the page's first Vendor_OnDraw.
 * With shopsanity off the apworld locks each location's vanilla card / password there, so all of this still runs and
 * the items shown are the vanilla ones.
 */
static u32 gClsVendorItem, gClsVIStudent, gClsStudent;
static int offVM = -1, offVMItem = -1, offPgVM = -1, offPgImg = -1, offPgL0 = -1, offPgL1 = -1, offImgImage = -1,
           offLblCaption = -1, offVICur = -1, offCardCur = -1, offShHidden = -1;
static u32 maskShHidden;
static int N_BeginState = -1, N_Timer = -1, N_VendorOnDraw = -1, N_StDoing = -1, N_StPost = -1, N_StSale = -1;
static int N_Touch = -1, N_ValidTouch = -1, N_Announce = -1, N_SetRespawn = -1, N_StPickup = -1, offShowHUD = -1;
static u32 maskShowHUD;
static volatile int gShReady;
static fstr_assign_fn gShFStr;

/* stands on the current map (worker scan, sorted by name so the assignment is stable) */
#define SH_MAXST 8
static volatile u32 gFG[SH_MAXST], gPW[SH_MAXST]; static volatile int gNFG, gNPW; static volatile u32 gShLevel;
static int gFGLi[SH_MAXST]; static u32 gFGHold[SH_MAXST], gPWHold[SH_MAXST];
/* card table cache (game thread) */
static u32 gShCards; static int gShN, gShRecvN = -1; static short gShIdx[AP_NSHOP], gShLi[AP_NSHOP]; static int gShLiInit;
static unsigned char gShRecv[96];
static int gVendRow[21];   /* purchase number -> AP_SHOP row */
/* transactions (game thread) */
static int gVendK = -1, gVendDlgLi = -1, gVendTie, gVendTieLi = -1; static u32 gVendTieT;
static int gPwSwap, gPwSwapK = -1; static u32 gPwSwapT;
static u32 gShTxnStand, gPgDone;
/* SHOPTEXT from the client (worker writes, game thread reads) */
static wchar gShName[AP_NSHOP][64], gShDesc[AP_NSHOP][160];

/* ---- worker -------------------------------------------------------------------------------------------------------- */
static void apshop_init(void) {
    static u32 last; static wchar b[400];
    if (gShReady) return;
    if (last && GetTickCount() - last < 3000) return;
    last = GetTickCount();
    if (!shop_init()) return;
    if (!gClsVendorItem) gClsVendorItem = find_class(L"VendorItem");
    if (!gClsVIStudent) gClsVIStudent = find_class(L"VI_StudentWizardCard");
    if (!gClsStudent) gClsStudent = find_class(L"HPGenericStudent");
    if (!gClsVendorItem) return;   /* HGame not loaded yet */
    offVM = prop_offset(L"HGame", L"VendorManager"); offVMItem = prop_offset(L"VendorManager", L"item");
    offPgVM = prop_offset(L"HPPageVendor", L"VendorMgr"); offPgImg = prop_offset(L"HPPageVendor", L"imgItem");
    offPgL0 = prop_offset(L"HPPageVendor", L"lblInfo0"); offPgL1 = prop_offset(L"HPPageVendor", L"lblInfo1");
    offImgImage = prop_offset(L"GUIImage", L"Image"); offLblCaption = prop_offset(L"GUILabel", L"Caption");
    offVICur = prop_offset(L"VendorItem", L"CurrencyType"); offCardCur = struct_member_offset(L"CardDesc", L"Currency");
    if (offCardStatus < 0) offCardStatus = struct_member_offset(L"CardDesc", L"Status");
    ch_bool(L"bHidden", &offShHidden, &maskShHidden);
    N_BeginState = name_index(L"BeginState"); N_Timer = name_index(L"Timer"); N_VendorOnDraw = name_index(L"Vendor_OnDraw");
    N_StDoing = name_index(L"DoingVendorTransaction"); N_StPost = name_index(L"PostTransaction");
    N_StSale = name_index(L"SaleAccepted");
    N_ValidTouch = name_index(L"ValidTouch"); N_Announce = name_index(L"AnnouncePickup"); N_SetRespawn = name_index(L"SetRespawn");
    N_StPickup = name_index(L"Pickup");
    {   u32 bp = prop_object(L"KWPickup", L"bShowHUD"), m = bp ? rd32(bp + 0x70) : 0;
        if (bp && m && !(m & (m - 1))) { offShowHUD = (int)rd32(bp + 0x4C); maskShowHUD = m; } }
    if (N_ValidTouch >= 0 && N_Announce >= 0 && N_SetRespawn >= 0 && N_StPickup >= 0 && offShowHUD > 0) N_Touch = name_index(L"Touch");
    {   static wchar t[160]; wsprintfW(t, L"shopsanity: pickup takeover names %d %d %d %d, bShowHUD %d/%x -> %s", N_ValidTouch, N_Announce,
                                      N_SetRespawn, N_StPickup, offShowHUD, maskShowHUD, N_Touch >= 0 ? L"on" : L"OFF (vanilla reward taken back instead)");
        log_line(t); }
    {   HANDLE core = GetModuleHandleW(L"Core.dll");
        gShFStr = core ? (fstr_assign_fn)GetProcAddress(core, "??4FString@@QAEAAV0@PBG@Z") : 0; }
    /* not the dialog's fields: HGame.HPPageVendor is only loaded when a sale dialog first opens (poc9j waited for
       them, so a session's first sale was a vanilla one); sh_page_lookups gets those */
    gShReady = gClsVIStudent && gClsStudent && offVM > 0 && offVMItem > 0 && offImgImage > 0 && offLblCaption > 0 && offVICur > 0 && offCardCur >= 0 && offCardStatus >= 0
        && offShHidden > 0 && maskShHidden && N_BeginState >= 0 && N_Timer >= 0 && N_StDoing >= 0 && N_StPost >= 0 && gShFStr
        && gSetCollision;
    wsprintfW(b, L"shopsanity: lookups vm=%d item=%d page vm=%d img=%d l0=%d l1=%d image=%d caption=%d cur=%d card.cur=%d "
              L"status=%d hidden=%d/%x names %d %d %d %d %d -> %s", offVM, offVMItem, offPgVM, offPgImg, offPgL0, offPgL1,
              offImgImage, offLblCaption, offVICur, offCardCur, offCardStatus, offShHidden, maskShHidden, N_BeginState,
              N_Timer, N_VendorOnDraw, N_StDoing, N_StPost, gShReady ? L"ready" : L"NOT READY (shops stay vanilla)");
    log_line(b);
}
struct shfind { u32 level; u32 fg[SH_MAXST], pw[SH_MAXST]; int fgn[SH_MAXST], pwn[SH_MAXST], nfg, npw; };
static void sh_ins(u32 *a, int *nm, int *n, u32 o, int name) {
    int i;
    if (*n >= SH_MAXST) return;
    for (i = *n; i > 0 && nm[i - 1] > name; i--) { a[i] = a[i - 1]; nm[i] = nm[i - 1]; }
    a[i] = o; nm[i] = name; (*n)++;
}
static int v_shstand(u32 o, int name, u32 cls, void *c) {
    struct shfind *f = c;
    if (!cls || rd32(o + offLevel) != f->level) return 0;
    if (is_a_cls(cls, gClsVIPw)) sh_ins(f->pw, f->pwn, &f->npw, o, name);
    else if (is_a_cls(cls, gClsVICard) && !is_a_cls(cls, gClsVIStudent) && !(gClsVIFinal && is_a_cls(cls, gClsVIFinal)))
        sh_ins(f->fg, f->fgn, &f->nfg, o, name);
    return 0;
}
/* worker: the sale dialog's fields, once its class is loaded (the game thread loads it: gShLoadPage) */
static u32 gShClsClass; static volatile int gShLoadPage;
static void sh_page_lookups(void) {
    static u32 last; static int asked; static wchar b[120];
    if (offPgVM > 0 && offPgImg > 0 && offPgL0 > 0 && offPgL1 > 0) return;
    if (last && GetTickCount() - last < 3000) return;
    last = GetTickCount();
    offPgVM = prop_offset(L"HPPageVendor", L"VendorMgr"); offPgImg = prop_offset(L"HPPageVendor", L"imgItem");
    offPgL0 = prop_offset(L"HPPageVendor", L"lblInfo0"); offPgL1 = prop_offset(L"HPPageVendor", L"lblInfo1");
    if (offPgVM > 0 && offPgImg > 0 && offPgL0 > 0 && offPgL1 > 0) {
        wsprintfW(b, L"shopsanity: sale dialog lookups vm=%d img=%d l0=%d l1=%d", offPgVM, offPgImg, offPgL0, offPgL1); log_line(b);
    } else if (!asked) {
        if (!gShClsClass) gShClsClass = find_class(L"Class");
        if (gShClsClass) { asked = 1; gShLoadPage = 1; }
    }
}
/* worker, every 250 ms (from ap_worker) */
static void apshop_worker(u32 level, int settle) {
    static u32 scanT; static wchar b[120]; struct shfind f; int i;
    if (settle <= 12) return;
    apshop_init();
    if (!gShReady) return;
    sh_page_lookups();
    if (level == gShLevel && GetTickCount() - scanT < 5000) return;
    scanT = GetTickCount();
    f.level = level; f.nfg = f.npw = 0;
    for_each_object(v_shstand, &f);
    if (level != gShLevel && (f.nfg || f.npw)) { wsprintfW(b, L"shopsanity: %d card stand(s), %d password stand(s) on this map", f.nfg, f.npw); log_line(b); }
    if (level != gShLevel) for (i = 0; i < SH_MAXST; i++) { gFGLi[i] = -1; gFGHold[i] = gPWHold[i] = 0; }
    for (i = 0; i < f.nfg; i++) gFG[i] = f.fg[i];
    for (i = 0; i < f.npw; i++) gPW[i] = f.pw[i];
    gNFG = f.nfg; gNPW = f.npw; gShLevel = level;
}
/* worker: SHOPTEXT <loc>|<name>|<description>; non-ASCII, '%' and '|' come as %XXXX (UTF-16, hex) */
static void sh_unescape(const char **p, wchar *out, int max) {
    int n = 0;
    while (**p && **p != '|') {
        wchar c; const char *s = *p;
        if (s[0] == '%' && s[1] && s[2] && s[3] && s[4]) {
            int k, v = 0;
            for (k = 1; k <= 4; k++) { char h = s[k]; v = v * 16 + (h >= '0' && h <= '9' ? h - '0' : h >= 'a' && h <= 'f' ? h - 'a' + 10 : h >= 'A' && h <= 'F' ? h - 'A' + 10 : 0); }
            c = (wchar)v; *p += 5;
        } else { c = (wchar)(unsigned char)s[0]; (*p)++; }
        if (n < max - 1) out[n++] = c;
    }
    out[n] = 0;
    if (**p == '|') (*p)++;
}
static void apshop_text(const char *p) {
    int off = a_num(&p), j;
    if (*p == '|') p++;
    for (j = 0; j < AP_NSHOP; j++) if (AP_SHOP[j].off == off) { sh_unescape(&p, gShName[j], 64); sh_unescape(&p, gShDesc[j], 160); return; }
}

/* ---- game thread --------------------------------------------------------------------------------------------------- */
static void sh_init_rows(void) {
    int j;
    if (gShLiInit) return;
    gShLiInit = 1;
    for (j = 0; j <= 20; j++) gVendRow[j] = -1;
    for (j = 0; j < AP_NSHOP; j++) {
        gShLi[j] = (short)ap_loc_index(AP_SHOP[j].off);
        if (AP_SHOP[j].kind == 'V' && AP_SHOP[j].n >= 1 && AP_SHOP[j].n <= 20) gVendRow[AP_SHOP[j].n] = j;
    }
}
static int sh_checked(int j) { int li = gShLi[j]; return li < 0 || gLocChecked[li] || gSrvChecked[li]; }
static int sh_row_of_li(int li) { int j; for (j = 0; j < AP_NSHOP; j++) if (gShLi[j] == li) return j; return -1; }
static int vendor_k(void) { int n; for (n = 1; n <= 20; n++) if (gVendRow[n] >= 0 && !sh_checked(gVendRow[n])) return gVendRow[n]; return -1; }
static int pw_k(void) { int j; for (j = 0; j < AP_NSHOP; j++) if (AP_SHOP[j].kind == 'W' && !sh_checked(j)) return j; return -1; }
static int sh_price(int j) { return gShopCheap > 0 ? cheap_of(AP_SHOP[j].cost) : AP_SHOP[j].cost; }

/* the card table: AP_SHOP rows -> card index (once per table), which cards were received (once per new item) */
static int sh_cards_cache(void) {
    int n, i, j; u32 cards = ap_cards(&n); wchar nm[64];
    if (!cards || n > 96) { gShCards = 0; return 0; }
    if (cards != gShCards || n != gShN) {
        gShCards = cards; gShN = n; gShRecvN = -1;
        for (j = 0; j < AP_NSHOP; j++) gShIdx[j] = -1;
        for (i = 0; i < n; i++) {
            u32 cl = P(cards + i * CARD_STRIDE + offCardCls);
            if (!cl || !name_of((int)P(cl + 0x20), nm, 64)) continue;
            for (j = 0; j < AP_NSHOP; j++) if (AP_SHOP[j].card && wieq(nm, AP_SHOP[j].card)) gShIdx[j] = (short)i;
        }
    }
    if (gRecvN != gShRecvN) { gShRecvN = gRecvN; for (i = 0; i < n; i++) gShRecv[i] = (unsigned char)ap_card_received(P(cards + i * CARD_STRIDE + offCardCls)); }
    return 1;
}
#define SH_CARD(i) (gShCards + (i) * CARD_STRIDE)
/* student vendors: one card on sale (Status 1) while a purchase is unchecked */
static void vendor_control(void) {
    int k = vendor_k(), sale = -1, i, j, changed = 0; static int lastSale = -2; static wchar b[160];
    if (!gShCards) return;
    if (k >= 0 && (sale = gShIdx[k]) >= 0 && gShRecv[sale]) {   /* k's own card was received: sell one that wasn't */
        for (i = 1; i <= 20; i++) if ((j = gVendRow[i]) >= 0 && gShIdx[j] >= 0 && !gShRecv[gShIdx[j]]) { sale = gShIdx[j]; break; }
    }
    for (i = 0; i < gShN; i++) {
        u8 *st = (u8 *)(SH_CARD(i) + offCardStatus);
        if (i == sale) { if (*st != 1) { *st = 1; changed++; } }
        else if (*st == 1) { *st = gShRecv[i] ? 5 : 6; changed++; }
    }
    gVendK = k;
    if (sale != lastSale || changed > 1) {
        lastSale = sale;
        wsprintfW(b, L"shopsanity: vendors sell %s (card %d, %d status change(s))", k >= 0 ? AP_LOCS[gShLi[k]].name : L"nothing more", sale, changed);
        log_line(b);
    }
}
static void sh_show(u32 a, int on) {
    u32 *w = (u32 *)(a + offShHidden); int hid = (*w & maskShHidden) != 0;
    if (on && hid) { *w &= ~maskShHidden; gSetCollision((void *)a, 1, 1, 1); }
    else if (!on && !hid) { *w |= maskShHidden; gSetCollision((void *)a, 0, 0, 0); }
}
static void fg_stands(void) {
    int U[8], nu = 0, i, j; u32 now = GetTickCount();
    for (j = 0; j < AP_NSHOP && nu < 8; j++) if (AP_SHOP[j].kind == 'C' && !sh_checked(j)) U[nu++] = j;
    for (i = 0; i < gNFG; i++) {
        u32 a = gFG[i], cls; int idx;
        if (!a || a == gShTxnStand || (gFGHold[i] && (int)(now - gFGHold[i]) < 0)) continue;
        if (i < nu && (idx = gShIdx[U[i]]) >= 0 && (cls = P(SH_CARD(idx) + offCardCls)) != 0) {
            j = U[i];
            if (gFGLi[i] != gShLi[j]) { static wchar b[160]; wsprintfW(b, L"shopsanity: card stand %d sells %s", i, AP_LOCS[gShLi[j]].name); log_line(b); }
            *(u32 *)(a + offVICardCls) = cls;
            *(int *)(a + offVICost) = (int)P(SH_CARD(idx) + offCardCost);
            *(u8 *)(a + offVICur) = *(u8 *)(SH_CARD(idx) + offCardCur);
            sh_show(a, 1);
            gFGLi[i] = gShLi[j];
            ap_look_set(a, gShLi[j], 1.0f, 0);
        } else {
            sh_show(a, 0); gFGLi[i] = -1; ap_look_forget(a);
        }
    }
}
static void pw_stands(void) {
    int k = pw_k(), i; u32 now = GetTickCount();
    for (i = 0; i < gNPW; i++) {
        u32 a = gPW[i];
        if (!a || a == gShTxnStand || (gPWHold[i] && (int)(now - gPWHold[i]) < 0)) continue;
        if (k >= 0) { sh_show(a, 1); ap_look_set(a, gShLi[k], 1.0f, 2); }
        else { sh_show(a, 0); ap_look_forget(a); }
    }
}
static u32 pw_table(void) {
    u32 pc = gPlayer ? P(gPlayer + offPlayerActor) : 0, level = pc ? P(pc + offLevel) : 0;
    u32 game = level && offGame > 0 ? P(level + offGame) : 0, ppm = game ? P(game + offPPM) : 0;
    return ppm && (int)P(ppm + offPWs + 4) == 22 ? P(ppm + offPWs) : 0;
}
static void pw_swap_begin(void) {
    int k = pw_k(), j; u32 pws = pw_table();
    if (!pws || k < 0) return;
    for (j = 0; j < AP_NSHOP; j++) if (AP_SHOP[j].kind == 'W' && AP_SHOP[j].n < 8)
        *(u32 *)(pws + AP_SHOP[j].n * PW_STRIDE + offPwOwned) = sh_checked(j) ? 1 : 0;
    gPwSwap = 1; gPwSwapK = k; gPwSwapT = GetTickCount();
}
static void pw_restore(void) { gPwSwap = 0; gPwSwapK = -1; portraits_apply(); }

static int sh_active(void) { return gShReady && gApActive && gRecvSynced; }
/* game thread, from ap_tick */
static void apshop_tick(void) {
    static u32 last;
    if (gShLoadPage && gLoadObject && gShClsClass) {   /* so the worker can find the sale dialog's fields */
        gShLoadPage = 0;
        log_line(gLoadObject((void *)gShClsClass, 0, L"HGame.HPPageVendor", 0, 0x2002, 0) ? L"shopsanity: sale dialog class loaded"
                                                                                       : L"shopsanity: sale dialog class not found");
    }
    if (!sh_active()) return;
    if (gPwSwap && GetTickCount() - gPwSwapT > 120000) { pw_restore(); log_line(L"shopsanity: password sale timed out; flags put back"); }
    if (gVendTie && GetTickCount() - gVendTieT > 60000) gVendTie = 0;
    if (GetTickCount() - last < 300) return;
    last = GetTickCount();
    sh_init_rows();
    if (!sh_cards_cache()) return;
    vendor_control();
    if (gShLevel == cur_level() && !gPwSwap) { fg_stands(); pw_stands(); }
}
/* pe_hook: BeginState. Returns 1 if it ran the event itself. */
static int apshop_begin_state(void *self, void *func, void *parms, void *result) {
    u32 o = (u32)self, st; int stn, i;
    if (!sh_active() || !func) return 0;
    st = P((u32)func + 0x18);   /* the state the BeginState belongs to */
    if (!st) return 0;
    stn = (int)P(st + 0x20);
    if (stn == N_StSale && N_StSale >= 0) {
        /* the twins' card stand is about to run Purchased: no card class, so the WizardCardInvItem it gives owns
           nothing (MarkWizardCardAsOwned(None) changes no card) and the card count doesn't move */
        u32 item = offVMItem > 0 ? P(o + offVMItem) : 0;
        for (i = 0; item && i < gNFG; i++) if (gFG[i] == item && gFGLi[i] >= 0) *(u32 *)(item + offVICardCls) = 0;
        return 0;
    }
    if (stn != N_StDoing && stn != N_StPost) return 0;
    sh_init_rows();
    if (stn == N_StDoing && is_a_fast(o, gClsStudent)) {
        /* a student's sale: the card on sale costs purchase k's price while the transaction starts */
        int s = -1, k, oldCost = 0; u8 oldCur = 0; static wchar b[200];
        if (!sh_cards_cache()) return 0;
        vendor_control();
        k = gVendK;
        for (i = 0; i < gShN; i++) if (*(u8 *)(SH_CARD(i) + offCardStatus) == 1) { s = i; break; }
        if (s >= 0 && k >= 0) {
            oldCost = *(int *)(SH_CARD(s) + offCardCost); oldCur = *(u8 *)(SH_CARD(s) + offCardCur);
            *(int *)(SH_CARD(s) + offCardCost) = sh_price(k); *(u8 *)(SH_CARD(s) + offCardCur) = AP_SHOP[k].cur;
            gVendTie = 1; gVendTieLi = gShLi[k]; gVendTieT = GetTickCount(); gVendDlgLi = gShLi[k];
            wsprintfW(b, L"shopsanity: student sale for %s (card %d at %d)", AP_LOCS[gShLi[k]].name, s, sh_price(k)); log_line(b);
        }
        gPEOrig(self, func, parms, result);
        if (s >= 0 && k >= 0) { *(int *)(SH_CARD(s) + offCardCost) = oldCost; *(u8 *)(SH_CARD(s) + offCardCur) = oldCur; }
        return 1;
    }
    if (!is_a_fast(o, gClsVendorItem)) return 0;
    if (stn == N_StDoing) {
        int ours = 0;
        for (i = 0; i < gNFG; i++) if (gFG[i] == o) ours = 1;
        for (i = 0; i < gNPW; i++) if (gPW[i] == o) { ours = 1; pw_swap_begin(); }
        if (ours) gShTxnStand = o;
        return 0;
    }
    /* PostTransaction: the sale is over (bought if Purchased hid the stand) */
    gPEOrig(self, func, parms, result);
    {   int bought = (P(o + offShHidden) & maskShHidden) != 0; static wchar b[200];
        if (gClsVIStudent && is_a_fast(o, gClsVIStudent)) { if (!bought) gVendTie = 0; return 1; }   /* declined: nothing thrown */
        for (i = 0; i < gNPW; i++) if (gPW[i] == o) {
            int k = gPwSwapK;
            if (bought && k >= 0) { gPWHold[i] = GetTickCount() + 4000; wsprintfW(b, L"shopsanity: bought %s", AP_LOCS[gShLi[k]].name); log_line(b); ap_check(gShLi[k]); }
            if (gPwSwap) pw_restore();
        }
        for (i = 0; i < gNFG; i++) if (gFG[i] == o) {
            if (bought && gFGLi[i] >= 0) {
                gFGHold[i] = GetTickCount() + 4000;
                wsprintfW(b, L"shopsanity: bought %s", AP_LOCS[gFGLi[i]].name); log_line(b);
                ap_check(gFGLi[i]);
                if (P(o + offVICardCls)) gApUndoCard = P(o + offVICardCls);   /* if Purchased did give a card: put it back */
                gFGLi[i] = -1;
            }
        }
        if (gShTxnStand == o) gShTxnStand = 0;
    }
    return 1;
}
/* pe_hook: Timer. Our stands don't reload themselves (Reload would pick the next vanilla card / password). */
static int apshop_timer(u32 o) {
    int i;
    if (!sh_active()) return 0;
    for (i = 0; i < gNFG; i++) if (gFG[i] == o) return 1;
    for (i = 0; i < gNPW; i++) if (gPW[i] == o) return 1;
    return 0;
}
/* ap.h: a pickup just appeared; the card a student throws after a sale stands for purchase k */
static int apshop_vendor_spawn(u32 obj) {
    int li; static wchar b[160];
    if (!gVendTie || GetTickCount() - gVendTieT > 60000 || ap_is_goodie(obj) != 3) return 0;
    li = gVendTieLi; gVendTie = 0;
    if (li < 0 || gLocChecked[li] || gApNPu >= AP_PUMAX) return 0;
    gApPu[gApNPu] = obj; gApPuLoc[gApNPu] = li; gApNPu++;
    ap_look_set(obj, li, 0.5f, 1);
    wsprintfW(b, L"shopsanity: the vendor's card stands for %s", AP_LOCS[li].name); log_line(b);
    return 1;
}

/* the dialog picture for a look code (HP3 menu art where there is some; else the 3D look's own texture) */
static const struct { unsigned char code; const wchar *tex; } DLGTEX[] = {
    { 1, L"SpellShapes.Shapes.SPSalohomora" }, { 2, L"SpellShapes.Shapes.SPSCarpeRetractum" },
    { 3, L"SpellShapes.Shapes.SPSdepulso" }, { 4, L"SpellShapes.Shapes.SPSdraconifor" },
    { 5, L"SpellShapes.Shapes.SPSexpecto" }, { 6, L"SpellShapes.Shapes.SPSglacius" },
    { 7, L"SpellShapes.Shapes.SPSlapifors" }, { 8, L"SpellShapes.Shapes.SPSlumos" },
    { 9, L"SpellShapes.Shapes.SPSrictusempra" }, { 10, L"SpellShapes.Shapes.SPSspongify" },
    { 20, L"HP3AP.APKeyDrawn" }, { 21, L"HP3_Menu.A_Menu.Hippogriff" }, { 22, L"HP3_Menu.A_Menu.Pixie" },
    { 23, L"HP3_Menu.A_Menu.MBOMPage" }, { 30, L"HP3_Menu.A_Misc.Scroll" }, { 32, L"HP3_Menu.A_Menu.A_Map_On" },
    { 33, L"HP3_Menu.A_Misc.PumpkinPasties" }, { 34, L"HP3_Menu.A_Misc.CauldronCakes" }, { 35, L"HP3_Menu.A_Misc.CocoFrog" },
    { 36, L"HP3_Menu.A_Menu.CardBack" }, { 40, L"HP3AP.APLogoProgression" }, { 41, L"HP3AP.APLogoUseful" },
    { 42, L"HP3AP.APLogoFiller" },
    /* portrait passwords 1-8: the portrait the twins' dialog shows for that password (PortraitPasswords[i].DescDlgTag) */
    { 51, L"HP3_Menu.A_Menu.Portrait_BertieBott" }, { 52, L"HP3_Menu.A_Menu.Portrait_DerwentShimpling" },
    { 53, L"HP3_Menu.A_Menu.Portrait_Mopsus" }, { 54, L"HP3_Menu.A_Menu.Portrait_TillyToke" },
    { 55, L"HP3_Menu.A_Menu.Portrait_GloverHipworth" }, { 56, L"HP3_Menu.A_Menu.Portrait_HesperStarkley" },
    { 57, L"HP3_Menu.A_Menu.Portrait_LeopoldinaSmethwyck" }, { 58, L"HP3_Menu.A_Menu.Portrait_BridgetWenlock" },
};
#define NDLGTEX (int)(sizeof DLGTEX / sizeof DLGTEX[0])
static u32 gDlgTex[NDLGTEX]; static unsigned char gDlgTried[NDLGTEX];
static u32 dlg_tex(int code) {
    int i;
    if (!gLoadObject || !gClsMaterial) return 0;
    if (code >= LOOK_CARD0 && code - LOOK_CARD0 < NWC_TEXTURES) {
        int c = code - LOOK_CARD0;
        if (!gCardTex[c] && !gCardTexTried[c]) { gCardTexTried[c] = 1; gCardTex[c] = load_kept((void *)gClsMaterial, WC_TEXTURES[c]); }
        return gCardTex[c];
    }
    for (i = 0; i < NDLGTEX; i++) if (DLGTEX[i].code == code) {
        if (!gDlgTex[i] && !gDlgTried[i]) {
            gDlgTried[i] = 1; gDlgTex[i] = load_kept((void *)gClsMaterial, DLGTEX[i].tex);
            if (!gDlgTex[i]) { static wchar b[160]; wsprintfW(b, L"shopsanity: dialog picture %s not found", DLGTEX[i].tex); log_line(b); }
        }
        return gDlgTex[i];
    }
    return 0;
}
/* pe_hook: HPPageVendor.Vendor_OnDraw (the page's OnDraw delegate), before it runs */
static void apshop_dialog(u32 page) {
    u32 vm, item, tex, img, l0, l1; int li = -1, i, j; static wchar b[200];
    if (page == gPgDone || !sh_active() || offPgVM <= 0 || offPgImg <= 0 || offPgL0 <= 0 || offPgL1 <= 0) return;
    gPgDone = page;
    vm = P(page + offPgVM); item = vm ? P(vm + offVMItem) : 0;
    if (!item) return;
    if (is_a_fast(item, gClsVIStudent)) li = gVendDlgLi;
    else if (gClsVIPw && is_a_fast(item, gClsVIPw)) li = gPwSwap && gPwSwapK >= 0 ? gShLi[gPwSwapK] : -1;
    else for (i = 0; i < gNFG; i++) if (gFG[i] == item) li = gFGLi[i];
    if (li < 0 || (j = sh_row_of_li(li)) < 0) return;
    img = P(page + offPgImg); l0 = P(page + offPgL0); l1 = P(page + offPgL1);
    if (img && gLook[li] && (tex = dlg_tex(gLook[li])) != 0) *(u32 *)(img + offImgImage) = tex;
    if (gShName[j][0]) {
        if (l0) gShFStr((void *)(l0 + offLblCaption), gShName[j]);
        if (l1) gShFStr((void *)(l1 + offLblCaption), gShDesc[j]);
    }
    wsprintfW(b, L"shopsanity: dialog shows %s (look %d)", AP_LOCS[li].name, gLook[li]); log_line(b);
}

/* ---- pickups: a location's pickup gives nothing ---------------------------------------------------------------------
   The vanilla Touch (Engine.Pickup, state Pickup) gives the item (SpawnCopy: inventory / card ownership) and
   AnnouncePickup flashes its HUD counter (HandlePickup, when bShowHUD). ap.h used to take the reward back a tick later,
   so the counter went up and down again. For a pickup that stands for a location the mod runs the Touch itself:
   ValidTouch (same rules), AnnouncePickup with bShowHUD off (HasBeenPickedUp, its save key, the sound and sparkle),
   SetRespawn (Destroy). Destroyed then checks the location as before (ap.h), with nothing to take back. */
static int ap_pickup_loc(u32 obj) {
    int i;
    for (i = 0; i < gApNPu; i++) if (gApPu[i] == obj) return gApPuLoc[i];
    for (i = 0; i < gApMapN; i++) if (gApMapObj[i] == obj && AP_LOCS[gApMapLoc[i]].kind != 'S') return gApMapLoc[i];
    return -1;
}
/* pe_hook: Touch. Returns 1 if it took the event over. */
static int ap_touch(void *self, void *func, void *parms) {
    u32 o = (u32)self, st; int li; void *fn; struct { u32 other, ret; } vt; struct { u32 receiver; } an; u32 *w;
    if (!gApActive || !func || !parms || !gClsPickup || !is_a_fast(o, gClsPickup)) return 0;
    st = P((u32)func + 0x18);
    if (!st || (int)P(st + 0x20) != N_StPickup) return 0;   /* only the giving Touch (state Pickup and its children) */
    if ((li = ap_pickup_loc(o)) < 0 || ap_loc_off(li)) return 0;
    if (!(fn = gFindFunction(self, N_ValidTouch, 0))) return 0;
    vt.other = *(u32 *)parms; vt.ret = 0;
    gPEOrig(self, fn, &vt, 0);
    if (!vt.ret) return 1;   /* not a valid touch: nothing happens, as vanilla */
    if (!(fn = gFindFunction(self, N_Announce, 0))) return 0;
    w = (u32 *)(o + offShowHUD); *w &= ~maskShowHUD;
    an.receiver = *(u32 *)parms;
    gPEOrig(self, fn, &an, 0);
    gApTaken[gApTakenN] = o; gApTakenN = (gApTakenN + 1) & 15;
    if ((fn = gFindFunction(self, N_SetRespawn, 0))) gPEOrig(self, fn, 0, 0);   /* Destroy -> Destroyed -> the check */
    return 1;
}
