/*
 * Shop prices (YAML option shop_prices: normal | cheap). Test switch until slot data exists:
 * Documents\Harry Potter and the Prisoner of Azkaban\HP3_OpenCastle.ini, [Shop] Cheap=1 (default 0 = normal), read
 * every 2 s so it can be flipped while the game runs.
 *
 * Cheap = 10% of the vanilla price, rounded up (25 pasties -> 3). It applies to the purchases that are checks: wizard
 * cards and portrait passwords. Pasties, cakes and chocolate frogs bought for jellybeans stay vanilla (they are
 * currency, not checks; 3-bean pasties would make the pasty-priced cards trivial).
 *
 * Where the prices live (HGame, decompiled):
 *   WizardCardManager.Cards[i].cost          (struct CardDesc; read by VI_WizardCard.GetCostAndCurrency and by the
 *                                             student vendors, HPGenericStudent.DoingVendorTransaction, at sale time)
 *   PortraitPasswordManager.PortraitPasswords[i].cost   (VI_PortraitPassword.LoadCostAndCurrency, next unowned 0..7)
 *   VendorItem.cost                          (what VendorManager checks, deducts and HPPageVendor shows; the stands
 *                                             copy it from the managers when the map loads and after each sale)
 * Both managers are rebuilt from class defaults on every map load. So, every 2 s on a settled map, the worker writes
 * the wanted price into the manager tables (only over a value that is the vanilla or the cheap one, and only if the
 * card class at that index is the expected one), then copies each stand's price from the tables again, the way
 * the stand itself would.
 */
typedef unsigned short u16;
static const struct { const wchar *cls; u16 cost; } VANILLA_CARDS[80] = {
    { L"WC_AlgufftheAwful", 100 }, { L"WC_AmarilloLestoat", 100 }, { L"WC_Babayaga", 100 },
    { L"WC_BeatrixBloxam", 25 }, { L"WC_Billywig", 100 }, { L"WC_BlodwynBludd", 100 },
    { L"WC_BowmanWright", 100 }, { L"WC_Bowtruckle", 100 }, { L"WC_BrantheBloodthirsty", 100 },
    { L"WC_CarlottaPinkstone", 100 }, { L"WC_Cliodne", 30 }, { L"WC_CommonWelshGreenDragon", 100 },
    { L"WC_CordeliaMisericordia", 100 }, { L"WC_CountVladDrakul", 100 }, { L"WC_CrispinCronk", 100 },
    { L"WC_Cyclops", 100 }, { L"WC_CyprianYoudle", 100 }, { L"WC_DorcasWellbeloved", 175 },
    { L"WC_DoubleEndedNewt", 100 }, { L"WC_Doxy", 100 }, { L"WC_Dumbledore", 100 },
    { L"WC_DymphnaFurmage", 50 }, { L"WC_EargittheUgly", 100 }, { L"WC_EdgarStroulger", 150 },
    { L"WC_ElfridaClagg", 125 }, { L"WC_EthelredTheEverReady", 80 }, { L"WC_FalcoAesalon", 175 },
    { L"WC_FulberttheFearful", 100 }, { L"WC_GiantPurpleToad", 100 }, { L"WC_GiantSquid", 100 },
    { L"WC_GlanmorePeakes", 60 }, { L"WC_Gnome", 100 }, { L"WC_GodricGryffindor", 100 },
    { L"WC_Goliath", 100 }, { L"WC_Gringott", 100 }, { L"WC_GroganStump", 25 },
    { L"WC_GunhildaOfGorsemoor", 100 }, { L"WC_GwenogJones", 100 }, { L"WC_Gytrash", 100 },
    { L"WC_HarryPotter", 100 }, { L"WC_HebrideanBlackDragon", 100 }, { L"WC_HelgaHufflepuff", 100 },
    { L"WC_HengistofUpperBarnton", 100 }, { L"WC_HesperStarkey", 125 }, { L"WC_HonoriaNutcombe", 175 },
    { L"WC_HungarianHorntailDragon", 100 }, { L"WC_IgnatiaWildsmith", 10 }, { L"WC_Imp", 100 },
    { L"WC_JocundaSykes", 200 }, { L"WC_JoscelindWadcock", 100 }, { L"WC_Kelpie", 100 },
    { L"WC_LadyCarmillaSanguina", 100 }, { L"WC_LeticiaSomnolens", 100 }, { L"WC_MalodoraGrymm", 100 },
    { L"WC_Manticore", 100 }, { L"WC_MirabellaPlunkett", 25 }, { L"WC_MorganLeFay", 50 },
    { L"WC_Morholt", 100 }, { L"WC_MountainTroll", 150 }, { L"WC_MungoBonham", 50 },
    { L"WC_NorvelTwonk", 125 }, { L"WC_NorwegianRidgebackDragon", 100 }, { L"WC_OldMotherHubbard", 100 },
    { L"WC_OswaldBeamish", 225 }, { L"WC_Phoenix", 100 }, { L"WC_ProfessorAmandoDippet", 250 },
    { L"WC_RoderickPlumpton", 100 }, { L"WC_RomanianLonghornDragon", 100 }, { L"WC_RowenaRavenclaw", 100 },
    { L"WC_SacharissaTugwood", 200 }, { L"WC_SalazarSlytherin", 100 }, { L"WC_SirHerbertVarney", 100 },
    { L"WC_Streeler", 200 }, { L"wc_ThaddeusThurkell", 70 }, { L"WC_UgtheUnreliable", 100 },
    { L"WC_Unicorn", 100 }, { L"WC_UrgtheUnclean", 100 }, { L"WC_WendelintheWeird", 75 },
    { L"WC_WilfredElphick", 40 }, { L"WC_ZouYen", 20 },
};
static const u16 VANILLA_PW[22] = { 10, 12, 12, 15, 15, 20, 25, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#define NVCARDS (int)(sizeof VANILLA_CARDS / sizeof VANILLA_CARDS[0])
#define NVPW    (int)(sizeof VANILLA_PW / sizeof VANILLA_PW[0])
#define SHOP_PWSOLD 8          /* PortraitPasswordManager sells 0..7 (FIRST_PORTRAIT_SHORTCUT_INDEX = 8) */
#define SHOP_MAXVI  32

static int cheap_of(int v) { return (v + 9) / 10; }   /* 10%, rounded up */
static int offWCM = -1, offCards = -1, offPWs = -1, offCardCost = -1, offCardCls = -1, offPwCost = -1, offPwOwned = -1;
static int offVICost = -1, offVICardCls = -1, gCardStride, gPwStride;
static u32 gClsVICard, gClsVIPw, gClsVIFinal;
static int gShopCheap = -1;
static u32 gShopLevel, gShopVI[SHOP_MAXVI]; static int gShopNVI;
static u32 gShopCardsChecked, gShopPwChecked;   /* table data pointers already verified */
static int gShopCardsOk, gShopPwOk;

/* struct sizes: CardDesc {byte Currency; int cost; int GameStateAvailable; byte Status; class CardClass} = 20,
   PortraitPasswordDesc {byte Currency; int cost; string DescDlgTag; string PasswordDlgTag; int Owned} = 36 (4-byte
   alignment, FString 12). Not read from the engine: each table is checked against the retail values before any write
   (card class names; password prices), so a wrong guess only switches the feature off. */
#define CARD_STRIDE 20
#define PW_STRIDE   36
static int shop_init(void) {
    static int state; static wchar b[300];
    if (state) return state > 0;
    offWCM = prop_offset(L"HGame", L"WizardCardManager");
    offCards = prop_offset(L"WizardCardManager", L"Cards"); offPWs = prop_offset(L"PortraitPasswordManager", L"PortraitPasswords");
    offCardCost = struct_member_offset(L"CardDesc", L"cost"); offCardCls = struct_member_offset(L"CardDesc", L"CardClass");
    offPwCost = struct_member_offset(L"PortraitPasswordDesc", L"cost"); offPwOwned = struct_member_offset(L"PortraitPasswordDesc", L"Owned");
    offVICost = prop_offset(L"VendorItem", L"cost"); offVICardCls = prop_offset(L"VI_WizardCard", L"WizardCardClass");
    gCardStride = CARD_STRIDE; gPwStride = PW_STRIDE;
    gClsVICard = find_class(L"VI_WizardCard"); gClsVIPw = find_class(L"VI_PortraitPassword"); gClsVIFinal = find_class(L"VI_FinalWizardCard");
    if (offWCM < 0 && offCards < 0) return 0;   /* HGame not loaded yet: try again later */
    state = (offWCM > 0 && offPPM > 0 && offCards > 0 && offPWs > 0 && offCardCost >= 0 && offCardCls >= 0 && offPwCost >= 0
             && offPwOwned >= 0 && offVICost > 0 && offVICardCls > 0 && gClsVICard && gClsVIPw) ? 1 : -1;
    wsprintfW(b, L"shop: lookups wcm=%d ppm=%d cards=%d pws=%d card.cost=%d card.class=%d pw.cost=%d pw.owned=%d vi.cost=%d vi.class=%d "
              L"stride %d/%d classes %s -> %s", offWCM, offPPM, offCards, offPWs, offCardCost, offCardCls, offPwCost, offPwOwned,
              offVICost, offVICardCls, gCardStride, gPwStride, gClsVICard && gClsVIPw ? L"ok" : L"MISSING", state > 0 ? L"ok" : L"shop prices DISABLED");
    log_line(b);
    return state > 0;
}
/* write `want` over `cur` only if cur is one of the two prices we know for that slot */
static int set_price(u32 addr, int vanilla, int cheap) {
    int cur = (int)rd32(addr), want = cheap ? cheap_of(vanilla) : vanilla;
    if (cur == want || (cur != vanilla && cur != cheap_of(vanilla))) return 0;
    *(volatile int *)addr = want; return 1;
}
struct vifind { u32 level; };
static int v_shopvi(u32 o, int name, u32 cls, void *c) {
    struct vifind *f = c; (void)name;
    if (!cls || gShopNVI >= SHOP_MAXVI || rd32(o + offLevel) != f->level) return 0;
    if ((is_a_cls(cls, gClsVICard) && !(gClsVIFinal && is_a_cls(cls, gClsVIFinal))) || is_a_cls(cls, gClsVIPw)) gShopVI[gShopNVI++] = o;
    return 0;
}
static void shop_poll(u32 level, u32 game) {
    static wchar b[200]; static u32 scanTick; int cheap, i, n = 0;
    u32 wcm, ppm, cards, pws; int ncards, npws;
    cheap = GetPrivateProfileIntW(L"Shop", L"Cheap", 0, gIniPath) != 0;
    if (cheap != gShopCheap) { gShopCheap = cheap; wsprintfW(b, L"shop prices: %s", cheap ? L"cheap (10%, rounded up)" : L"normal (vanilla)"); log_line(b); }
    if (!game || !shop_init()) return;
    wcm = rd32(game + offWCM); ppm = rd32(game + offPPM);
    cards = wcm ? rd32(wcm + offCards) : 0; ncards = wcm ? (int)rd32(wcm + offCards + 4) : 0;
    pws = ppm ? rd32(ppm + offPWs) : 0; npws = ppm ? (int)rd32(ppm + offPWs + 4) : 0;
    if (cards && cards != gShopCardsChecked) {   /* new table: check every class name against ours once */
        wchar nm[48]; int bad = -1;
        gShopCardsChecked = cards; gShopCardsOk = ncards == NVCARDS;
        for (i = 0; gShopCardsOk && i < NVCARDS; i++) {
            u32 cl = rd32(cards + i * gCardStride + offCardCls);
            if (!cl || !name_of((int)rd32(cl + 0x20), nm, 48) || !wieq(nm, VANILLA_CARDS[i].cls)) { gShopCardsOk = 0; bad = i; }
        }
        if (!gShopCardsOk) { wsprintfW(b, L"shop: wizard card table differs from the retail one (%d cards, first mismatch %d); card prices left alone", ncards, bad); log_line(b); }
    }
    if (pws && pws != gShopPwChecked) {   /* new table: every price must be the retail one (or our cheap one), owned 0/1 */
        gShopPwChecked = pws; gShopPwOk = npws == NVPW;
        for (i = 0; gShopPwOk && i < NVPW; i++) {
            int c = (int)rd32(pws + i * gPwStride + offPwCost); u32 o = rd32(pws + i * gPwStride + offPwOwned);
            if ((c != VANILLA_PW[i] && c != cheap_of(VANILLA_PW[i])) || o > 1) gShopPwOk = 0;
        }
        if (!gShopPwOk) { wsprintfW(b, L"shop: password table differs from the retail one (%d entries); password prices left alone", npws); log_line(b); }
    }
    if (gShopCardsOk) for (i = 0; i < NVCARDS; i++) n += set_price(cards + i * gCardStride + offCardCost, VANILLA_CARDS[i].cost, cheap);
    if (gShopPwOk) for (i = 0; i < SHOP_PWSOLD; i++) n += set_price(pws + i * gPwStride + offPwCost, VANILLA_PW[i], cheap);
    if (n) { wsprintfW(b, L"shop: %d table price(s) set to %s", n, cheap ? L"cheap" : L"normal"); log_line(b); }
    /* the stands on this map (rescanned on a new map and every 20 s: student vendors spawn theirs at sale time and read
       the table then, so they don't need it, but a rescan is cheap insurance) */
    if (level != gShopLevel || GetTickCount() - scanTick > 20000) {
        struct vifind f; f.level = level; gShopNVI = 0; for_each_object(v_shopvi, &f);
        if (level != gShopLevel && gShopNVI) { wsprintfW(b, L"shop: %d card/password stand(s) on this map", gShopNVI); log_line(b); }
        gShopLevel = level; scanTick = GetTickCount();
    }
    n = 0;
    for (i = 0; i < gShopNVI; i++) {
        u32 vi = gShopVI[i], cls = rd32(vi + 0x24); int want = -1, k;
        if (rd32(vi + offLevel) != level) continue;
        if (is_a_cls(cls, gClsVIPw)) {   /* next unowned password, as LoadCostAndCurrency */
            if (!gShopPwOk) continue;
            for (k = 0; k < SHOP_PWSOLD; k++) if (rd32(pws + k * gPwStride + offPwOwned) == 0) break;
            if (k < SHOP_PWSOLD) want = (int)rd32(pws + k * gPwStride + offPwCost);
        } else if (gShopCardsOk) {       /* its card's table price, as GetCostAndCurrency */
            u32 wc = rd32(vi + offVICardCls);
            for (k = 0; wc && k < NVCARDS; k++) if (rd32(cards + k * gCardStride + offCardCls) == wc) { want = (int)rd32(cards + k * gCardStride + offCardCost); break; }
        }
        if (want >= 0 && (int)rd32(vi + offVICost) != want) { *(volatile int *)(vi + offVICost) = want; n++; }
    }
    if (n) { wsprintfW(b, L"shop: %d stand price(s) updated", n); log_line(b); }
}
