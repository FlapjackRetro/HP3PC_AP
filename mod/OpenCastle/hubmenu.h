/*
 * Softlock escape: a "RETURN TO HUB" button in the in-game exit dialog (pause menu -> X -> "Exit game and return to
 * Main Menu?", class HGame.HPPageExit).
 *
 * The dialog already owns a spare button: HPPageQuit.decoy, an invisible GUIButton (no style) that only exists to
 * hold the initial focus. When the worker sees an HPPageExit become the active page, the game thread turns the decoy
 * into a third button under YES / NO: caption, NO's style, a position, and YES's OnClick / OnWatch delegates (so a
 * click goes to the page's MyOnClick, which the native GUI calls through ProcessEvent and our hook sees with
 * Sender = decoy). The decoy keeps the initial focus, so a stray Enter could click it: the first click only arms it
 * ("CLICK AGAIN TO RETURN TO HUB"), the second one closes the menus the way the menu's own X does
 * (HPPageMainMenu.ExitMenu: unpause, stop the menu music) and travels to InsideHub. The player arrives in the
 * entrance hall (the GroundsHub SmartStart is lent in TravelPostAccept, see travel_post_accept).
 * Needs from opencastle.c: gPEOrig, gFindFunction, log_line, gPendingURL/gPending, gHubReturn.
 */
static u32 gClsExitPage, gClsMainMenu;
static int offGUICtl = -1, offActivePage = -1, offDecoy = -1, offBtnYes = -1, offBtnNo = -1, offCaption = -1,
           offStyle = -1, offWinTop = -1, offWinLeft = -1, offWinWidth = -1, offWinHeight = -1, offOnClick = -1,
           offOnWatch = -1, offCompCtl = -1, offParentPage = -1, offBound = -1;
static u32 maskBound;   /* bit of GUIComponent.bBoundToParent in its dword, 0 if it couldn't be verified */
static int N_MainMenuCls = -1;   /* class name HPPageMainMenu (by name: menu classes may be reloaded) */
static int N_MyOnClick = -1, N_CloseMenu = -1, N_ExitMenu = -1, N_CloseAll = -1, N_SetPause = -1;
static volatile u32 gMenuPage; static volatile int gMenuPatch;
static u32 gHubPage; static int gHubArmed;
typedef void *(__thiscall *fstr_assign_fn)(void *self, const wchar *s);
static fstr_assign_fn gFStrAssign;

/* worker: the engine-side lookups, once. The menu classes (HGame.HPPageExit ...) are only loaded when a menu first
   opens, so those are looked up later, in hub_page_init, the first time an HPPageExit is the active page. */
static int hub_init(void) {
    u32 core, bprop;
    core = (u32)GetModuleHandleW(L"Core.dll");
    gFStrAssign = core ? (fstr_assign_fn)GetProcAddress((HANDLE)core, "??4FString@@QAEAAV0@PBG@Z") : 0;
    offGUICtl = prop_offset(L"Player", L"GUIController");
    offActivePage = prop_offset(L"GUIController", L"ActivePage");
    offCaption = prop_offset(L"GUIButton", L"Caption");
    offStyle = prop_offset(L"GUIComponent", L"Style");
    offWinTop = prop_offset(L"GUIComponent", L"WinTop"); offWinLeft = prop_offset(L"GUIComponent", L"WinLeft");
    offWinWidth = prop_offset(L"GUIComponent", L"WinWidth"); offWinHeight = prop_offset(L"GUIComponent", L"WinHeight");
    offOnClick = prop_offset(L"GUIComponent", L"__OnClick__Delegate");
    offOnWatch = prop_offset(L"GUIComponent", L"__OnWatch__Delegate");
    offCompCtl = prop_offset(L"GUI", L"Controller");
    offParentPage = prop_offset(L"GUIPage", L"ParentPage");
    bprop = prop_object(L"GUIComponent", L"bBoundToParent");
    if (bprop) { u32 m = rd32(bprop + 0x64); offBound = (int)rd32(bprop + 0x4C); if (m && !(m & (m - 1))) maskBound = m; }
    N_MyOnClick = name_index(L"MyOnClick"); N_CloseMenu = name_index(L"CloseMenu"); N_ExitMenu = name_index(L"ExitMenu");
    N_CloseAll = name_index(L"CloseAll"); N_SetPause = name_index(L"SetPause");
    {   static wchar b[300];
        wsprintfW(b, L"hub menu: engine lookups FString= %s gui=%d page=%d caption=%d style=%d click=%d watch=%d ctl=%d parent=%d bound=%d mask=%x",
                  gFStrAssign ? L"ok" : L"-", offGUICtl, offActivePage, offCaption, offStyle, offOnClick, offOnWatch, offCompCtl,
                  offParentPage, offBound, maskBound);
        log_line(b);
    }
    return gFStrAssign && offGUICtl > 0 && offActivePage > 0 && offCaption > 0 && offStyle > 0 && offWinTop > 0 && offWinLeft > 0
        && offWinWidth > 0 && offWinHeight > 0 && offOnClick > 0 && offOnWatch > 0 && offCompCtl > 0 && N_MyOnClick >= 0 && N_CloseMenu >= 0;
}
static int hub_page_init(u32 cls) {
    static int tried; static wchar b[200];
    if (gClsExitPage) { gClsExitPage = cls; return 1; }
    if (tried) return 0;
    tried = 1;
    offDecoy = prop_offset(L"HPPageQuit", L"decoy");
    offBtnYes = prop_offset(L"KWPageYesNo", L"btnYes"); offBtnNo = prop_offset(L"KWPageYesNo", L"btnNo");
    gClsMainMenu = find_class(L"HPPageMainMenu"); N_MainMenuCls = name_index(L"HPPageMainMenu");
    wsprintfW(b, L"hub menu: exit dialog lookups decoy=%d yes=%d no=%d mainmenu %s", offDecoy, offBtnYes, offBtnNo, gClsMainMenu ? L"ok" : L"-");
    log_line(b);
    if (offDecoy > 0 && offBtnYes > 0 && offBtnNo > 0) gClsExitPage = cls;
    return gClsExitPage != 0;
}
/* worker, every poll: while an HPPageExit is the active page and its decoy isn't our button (a new page, or the GUI
   re-ran InitComponent after a level change and put the decoy back), have the game thread patch it */
static void hub_poll(void) {
    static u32 lastActive; static int nExitPage = -2; u32 ctl, page, cls, decoy; wchar nm[48]; float w = 0;
    if (!gPlayer) return;
    if (nExitPage < 0) nExitPage = name_index(L"HPPageExit");
    ctl = rd32(gPlayer + offGUICtl); page = ctl ? rd32(ctl + offActivePage) : 0;
    cls = page ? rd32(page + 0x24) : 0;
    if (page != lastActive) {   /* diagnostics: every change of the active menu page */
        static wchar b[120];
        if (!cls || !name_of((int)rd32(cls + 0x20), nm, 48)) wcpy(nm, L"-", 48);
        wsprintfW(b, L"hub menu: active page %08x %s (controller %08x)", page, nm, ctl); log_line(b);
        lastActive = page;
    }
    /* by class name, not class pointer: a level change may reload the menu classes */
    if (!page || nExitPage < 0 || (int)rd32(cls + 0x20) != nExitPage || !hub_page_init(cls)) return;
    decoy = rd32(page + offDecoy);
    {   u32 yes = rd32(page + offBtnYes);
        if (decoy && yes && rd(decoy + offWinWidth, &w, 4) && w == 0.32f && rd32(decoy + offOnClick) == rd32(yes + offOnClick)
            && rd32(decoy + offOnClick + 4) == rd32(yes + offOnClick + 4)) return;   /* already our button */
    }
    if (!gMenuPatch) { gMenuPage = page; gMenuPatch = 1; }
}

static void set_caption(u32 btn, const wchar *s) { gFStrAssign((void *)(btn + offCaption), s); }
/* game thread */
static void hub_patch(void) {
    u32 page = gMenuPage, decoy, yes, no, *dy, *dd; static wchar msg[200];
    static u32 failedPage;
    if (page == failedPage) return;
    decoy = P(page + offDecoy); yes = P(page + offBtnYes); no = P(page + offBtnNo);
    if (!decoy || !yes || !no) { failedPage = page; log_line(L"hub menu: dialog has no decoy/yes/no button"); return; }
    /* YES's OnClick must point at this page's MyOnClick, or we don't know the delegate layout: leave the menu alone */
    dy = (u32 *)(yes + offOnClick);
    if (!((dy[0] == page && (int)dy[1] == N_MyOnClick) || ((int)dy[0] == N_MyOnClick && dy[1] == page))) {
        failedPage = page; wsprintfW(msg, L"hub menu: unexpected YES delegate %08x %08x; button not added", dy[0], dy[1]); log_line(msg); return;
    }
    dd = (u32 *)(decoy + offOnClick); dd[0] = dy[0]; dd[1] = dy[1];
    dy = (u32 *)(yes + offOnWatch); dd = (u32 *)(decoy + offOnWatch); dd[0] = dy[0]; dd[1] = dy[1];
    *(u32 *)(decoy + offStyle) = P(no + offStyle);
    set_caption(decoy, L"RETURN TO HUB");
    *(float *)(yes + offWinTop) = 0.5133f;
    *(float *)(no + offWinTop) = 0.5733f;
    if (maskBound && offBound >= 0 && (P(decoy + offBound) & maskBound) && !(P(no + offBound) & maskBound)) {
        *(u32 *)(decoy + offBound) &= ~maskBound;   /* screen coordinates, like YES / NO */
        *(float *)(decoy + offWinLeft) = 0.34f; *(float *)(decoy + offWinTop) = 0.6333f;
    } else {
        /* bound to the dialog: left/top are fractions of the dialog (0.18, 0.2867, 0.64 x 0.4267), width/height stay
           fractions of the screen (measured in game). Same column as YES / NO (0.34..0.66), under NO at 0.6333. */
        *(float *)(decoy + offWinLeft) = 0.25f; *(float *)(decoy + offWinTop) = 0.8123f;
    }
    *(float *)(decoy + offWinWidth) = 0.32f; *(float *)(decoy + offWinHeight) = 0.05333f;
    gHubPage = page; gHubArmed = 0;
    log_line(L"hub menu: RETURN TO HUB added to the exit dialog");
}
static void call0(u32 obj, int fname) { void *fn; if (obj && fname >= 0 && (fn = gFindFunction((void *)obj, fname, 0))) gPEOrig((void *)obj, fn, 0, 0); }
static void call_bool(u32 obj, int fname, u32 b) {
    struct { u32 b, ret; } parms; void *fn;
    if (obj && fname >= 0 && (fn = gFindFunction((void *)obj, fname, 0))) { parms.b = b; parms.ret = 0; gPEOrig((void *)obj, fn, &parms, 0); }
}
/* game thread, from pe_hook: page.MyOnClick(Sender). Returns 1 if it was our button (event handled). */
static int hub_click(u32 page, u32 *parms) {
    u32 decoy, ctl, parent, pc;
    if (!gHubPage || page != gHubPage || !parms) return 0;
    decoy = P(page + offDecoy);
    if (parms[0] != decoy) { gHubArmed = 0; return 0; }
    parms[1] = 1;   /* the delegate's bool result: handled */
    if (!gHubArmed) { gHubArmed = 1; set_caption(decoy, L"CLICK AGAIN TO RETURN TO HUB"); return 1; }
    gHubArmed = 0; set_caption(decoy, L"RETURN TO HUB");
    log_line(L"hub menu: RETURN TO HUB chosen");
    ctl = P(page + offCompCtl); parent = offParentPage >= 0 ? P(page + offParentPage) : 0;
    call_bool(ctl, N_CloseMenu, 0);                                   /* close this dialog */
    if (parent && N_MainMenuCls >= 0 && (int)P(P(parent + 0x24) + 0x20) == N_MainMenuCls) call0(parent, N_ExitMenu);   /* unpause, music, close */
    else {
        call_bool(ctl, N_CloseAll, 0);
        pc = gPlayer ? P(gPlayer + offPlayerActor) : 0;
        call_bool(pc, N_SetPause, 0);
    }
    gHubReturn = 1;
    wcpy(gPendingURL, L"HP3_InsideHub", 128); gPending = 1;
    return 1;
}
