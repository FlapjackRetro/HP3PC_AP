/*
 * Debug panel (TEST ONLY, remove from release builds: it is a cheat menu). Delete opens / closes it (backslash spawns the
 * old item test grid). While open the game is paused ([Debug] PanelPause=0 in the ini to leave it running) and:
 *   Up / Down       move              Left / Right   other column
 *   Enter / Space   tick / untick the highlighted unlock, or run the highlighted action
 * Each unlock's box shows its effective state; "(start)" means the ini forces it on (it can't be unticked here: set
 * it to 0 in the ini). Ticking writes the save flag (OC_*), so a debug setup is saved with the game.
 *
 * Drawn in the HUD's PostRender (the ProcessEvent hook runs the HUD's own PostRender first, then draws on its canvas)
 * with Engine.dll's UCanvas::DrawTile (a dark box from LevelInfo.WhiteSquareTexture) and UCanvas::WrappedPrintf.
 * Keys come from the key thread (GetAsyncKeyState, game window focused); everything else runs on the game thread.
 */
struct prow { char t; const wchar *label; int idx; const wchar *key; };
/* t: 'h' heading, '|' next column, 's' spell (SPELLS index), 'd' door (DOORS key), 'm' map, 'p' shortcut passwords,
   'w' portrait password (1-8), 'n' noclip, 'x' not wired yet, 'a' action */
static const struct prow PROWS[] = {
    { 'h', L"SPELLS", 0, 0 },
    { 's', L"Alohomora", 0, 0 }, { 's', L"Carpe Retractum", 1, 0 }, { 's', L"Depulso", 2, 0 }, { 's', L"Draconifors", 3, 0 },
    { 's', L"Expecto Patronum", 4, 0 }, { 's', L"Glacius", 5, 0 }, { 's', L"Lapifors", 6, 0 }, { 's', L"Lumos", 7, 0 },
    { 's', L"Rictusempra", 8, 0 }, { 's', L"Spongify", 9, 0 },
    { 'h', L"ITEMS", 0, 0 },
    { 'm', L"Marauder's Map", 0, 0 }, { 'p', L"Shortcut Passwords", 0, 0 },
    { 'h', L"ACTIONS", 0, 0 },
    { 'a', L"+10 Pumpkin Pasties", 0, 0 }, { 'a', L"+10 Cauldron Cakes", 1, 0 }, { 'a', L"+100 Jellybeans", 2, 0 },
    { 'a', L"Untick everything found (save flags off)", 3, 0 },
    { 'n', L"Noclip: fly through walls (F6; X up, C down, Shift fast)", 0, 0 },
    { '|', 0, 0, 0 },
    { 'h', L"LEVEL UNLOCKS", 0, 0 },
    { 'd', L"Paddock", 0, L"Paddock" }, { 'd', L"Hogwarts Express", 0, L"Express" }, { 'd', L"Patronus Lesson (DADA)", 0, L"DADA" },
    { 'd', L"Library", 0, L"Library" }, { 'd', L"Shrieking Shack", 0, L"Shack" },
    { 'd', L"Monster Book challenge", 0, L"MonsterBook" }, { 'd', L"Hippogriff challenge", 0, L"Hippogriff" },
    { 'd', L"Doxy challenge", 0, L"Doxy" }, { 'd', L"Wizard Card bonus room", 0, L"BonusRoom" },
    { 'd', L"Carpe Retractum challenge", 0, L"ChCarpe" }, { 'd', L"Draconifors / Lapifors challenge", 0, L"ChDracLap" },
    { 'd', L"Glacius challenge", 0, L"ChGlacius" },
    { 'd', L"Ron's exam (DADA)", 0, L"ExamRon" }, { 'd', L"Hermione's exam (Transfiguration)", 0, L"ExamHermione" },
    { 'd', L"Harry's exam (Charms)", 0, L"ExamHarry" },
    { 'h', L"PORTRAIT PASSWORDS (order the twins sell them)", 0, 0 },
    { 'w', L"Portrait Password 1", 0, 0 }, { 'w', L"Portrait Password 2", 1, 0 }, { 'w', L"Portrait Password 3", 2, 0 },
    { 'w', L"Portrait Password 4", 3, 0 }, { 'w', L"Portrait Password 5", 4, 0 }, { 'w', L"Portrait Password 6", 5, 0 },
    { 'w', L"Portrait Password 7", 6, 0 }, { 'w', L"Portrait Password 8", 7, 0 },
};
#define NPROWS (int)(sizeof PROWS / sizeof PROWS[0])
#define PK_UP 1
#define PK_DOWN 2
#define PK_LEFT 4
#define PK_RIGHT 8
#define PK_ENTER 16
/* gPanelOpen / gPanelPause (1 pause, 2 unpause) / gPanelKeys: aptest.h (the key thread) */
static volatile int gAddCurrency;                   /* bits: 0 pasties, 1 cakes, 2 beans (tick job) */
static int gSel = 1;
static int N_PostRender = -1; static u32 gClsHUD;
static int offCvFont = -1, offCvMed = -1, offCvSmall = -1, offCvCurX = -1, offCvCurY = -1, offCvOrgX = -1, offCvOrgY = -1,
           offCvClipX = -1, offCvClipY = -1, offCvColor = -1, offCvStyle = -1, offWhiteTex = -1, offVSize = -1,
           offColR = -1, offColG = -1, offColB = -1, offColA = -1;
typedef struct { float x, y, z, w; } fplane;
typedef void (__thiscall *drawtile_fn)(void *cv, void *mat, float x, float y, float xl, float yl, float u, float v,
                                      float ul, float vl, float z, fplane color, fplane fog);
typedef void (__cdecl *wprintf_fn)(void *cv, void *font, int center, const wchar *fmt, ...);
typedef void (__cdecl *wstrlen_fn)(void *cv, void *font, int *xl, int *yl, const wchar *fmt, ...);
static drawtile_fn gDrawTile; static wprintf_fn gWPrintf; static wstrlen_fn gWStrLen;
static int gPanelReady;   /* 1 ok, -1 lookups failed */

/* worker, once */
static void panel_lookups(void) {
    HANDLE eng; static wchar b[300]; static int tries;
    if (gPanelReady > 0 || tries >= 10) return;   /* a lookup can miss while a level loads (poc8o: Canvas.CurY): retry */
    tries++;
    N_PostRender = name_index(L"PostRender"); gClsHUD = find_class(L"HUD");
    offCvFont = prop_offset(L"Canvas", L"Font"); offCvMed = prop_offset(L"Canvas", L"MedFont"); offCvSmall = prop_offset(L"Canvas", L"SmallFont");
    offCvCurX = prop_offset(L"Canvas", L"CurX"); offCvCurY = prop_offset(L"Canvas", L"CurY");
    offCvOrgX = prop_offset(L"Canvas", L"OrgX"); offCvOrgY = prop_offset(L"Canvas", L"OrgY");
    offCvClipX = prop_offset(L"Canvas", L"ClipX"); offCvClipY = prop_offset(L"Canvas", L"ClipY");
    offCvColor = prop_offset(L"Canvas", L"DrawColor"); offCvStyle = prop_offset(L"Canvas", L"Style");
    offWhiteTex = prop_offset(L"LevelInfo", L"WhiteSquareTexture");
    if (offUSize < 0) offUSize = prop_offset(L"BitmapMaterial", L"USize");
    offVSize = prop_offset(L"BitmapMaterial", L"VSize");
    offColR = struct_member_offset(L"Color", L"R"); offColG = struct_member_offset(L"Color", L"G");
    offColB = struct_member_offset(L"Color", L"B"); offColA = struct_member_offset(L"Color", L"A");
    eng = GetModuleHandleW(L"Engine.dll");
    if (eng) {
        gDrawTile = (drawtile_fn)GetProcAddress(eng, "?DrawTile@UCanvas@@UAEXPAVUMaterial@@MMMMMMMMMVFPlane@@1@Z");
        gWPrintf = (wprintf_fn)GetProcAddress(eng, "?WrappedPrintf@UCanvas@@UAAXPAVUFont@@HPBGZZ");
        gWStrLen = (wstrlen_fn)GetProcAddress(eng, "?WrappedStrLenf@UCanvas@@UAAXPAVUFont@@AAH1PBGZZ");
    }
    gPanelReady = (N_PostRender >= 0 && gClsHUD && offCvFont > 0 && offCvCurX > 0 && offCvCurY > 0 && offCvOrgX > 0 && offCvOrgY > 0
                   && offCvClipX > 0 && offCvColor > 0 && offCvStyle > 0 && offColR >= 0 && offColG >= 0 && offColB >= 0 && offColA >= 0
                   && gWPrintf && gWStrLen) ? 1 : -1;
    wsprintfW(b, L"panel: lookups PostRender=%d hud %s canvas font=%d cur=%d/%d org=%d/%d clip=%d color=%d(%d%d%d%d) style=%d white=%d; "
              L"DrawTile %s WrappedPrintf %s StrLen %s -> %s", N_PostRender, gClsHUD ? L"ok" : L"-", offCvFont, offCvCurX, offCvCurY,
              offCvOrgX, offCvOrgY, offCvClipX, offCvColor, offColR, offColG, offColB, offColA, offCvStyle, offWhiteTex,
              gDrawTile ? L"ok" : L"-", gWPrintf ? L"ok" : L"-", gWStrLen ? L"ok" : L"-", gPanelReady > 0 ? L"ok" : L"PANEL DISABLED");
    if (gPanelReady > 0 || tries == 1 || tries >= 10) log_line(b);
    if (gPanelReady < 0 && tries < 10) gPanelReady = 0;   /* not ready yet: try again on the next poll */
}
static int door_index(const wchar *key) { int i; for (i = 0; i < NDOORS; i++) if (weq(DOORS[i].key, key)) return i; return -1; }
/* effective state and whether the ini forces it; -1 = not a toggle */
static int prow_state(const struct prow *r, int *forced) {
    int d;
    *forced = 0;
    switch (r->t) {
    case 's': *forced = gIniSpell[r->idx]; return gSpellOn[r->idx] != 0;
    case 'd': if ((d = door_index(r->key)) < 0) return -1; *forced = gIniDoor[d]; return gDoorOn[d] != 0;
    case 'm': *forced = gIniMap; return gHaveMap > 0;
    case 'p': *forced = gIniShortcuts; return gHaveShortcuts > 0;
    case 'w': *forced = gIniPortrait[r->idx]; return gHavePortrait[r->idx] > 0;
    case 'n': return gNoclip != 0;
    default: return -1;
    }
}
static int prow_selectable(int i) { return i >= 0 && i < NPROWS && PROWS[i].t != 'h' && PROWS[i].t != '|'; }
static int prow_col(int i) { int k, c = 0; for (k = 0; k < i; k++) if (PROWS[k].t == '|') c++; return c; }
static int prow_line(int i) { int k, n = 0; for (k = 0; k < i; k++) { if (PROWS[k].t == '|') n = 0; else n++; } return n; }

/* game thread: Enter on a row */
static void panel_activate(int i) {
    const struct prow *r = &PROWS[i]; static wchar b[160], k[16]; int forced, on = prow_state(r, &forced), d;
    if (r->t == 'a') {
        if (r->idx < 3) { gAddCurrency |= 1 << r->idx; return; }
        for (d = 0; d < NSPELLS; d++) if (gSaveSpell[d]) save_flag_put(L"Spells", SPELLS[d].key, 0);
        for (d = 0; d < NDOORS && d < 16; d++) if (gSaveDoor[d]) save_flag_put(L"Doors", DOORS[d].key, 0);
        if (gSaveMap) save_flag_put(L"Items", L"MaraudersMap", 0);
        if (gSaveShortcuts) save_flag_put(L"Items", L"ShortcutPasswords", 0);
        for (d = 0; d < 8; d++) if (gSavePortrait[d]) { wsprintfW(k, L"Portrait%d", d + 1); save_flag_put(L"Items", k, 0); }
        log_line(L"panel: every found flag cleared");
        return;
    }
    if (r->t == 'n') { gNoclip = !gNoclip; log_line(gNoclip ? L"panel: noclip on" : L"panel: noclip off"); return; }
    if (on < 0) { wsprintfW(b, L"panel: %s isn't wired up yet", r->label); log_line(b); return; }
    if (forced) { wsprintfW(b, L"panel: %s is on from the ini start state; set it to 0 there to turn it off", r->label); log_line(b); return; }
    switch (r->t) {
    case 's': save_flag_put(L"Spells", SPELLS[r->idx].key, !gSaveSpell[r->idx]); break;
    case 'd': if ((d = door_index(r->key)) >= 0) save_flag_put(L"Doors", DOORS[d].key, !gSaveDoor[d]); break;
    case 'm': save_flag_put(L"Items", L"MaraudersMap", !gSaveMap); break;
    case 'p': save_flag_put(L"Items", L"ShortcutPasswords", !gSaveShortcuts); break;
    case 'w': wsprintfW(k, L"Portrait%d", r->idx + 1); save_flag_put(L"Items", k, !gSavePortrait[r->idx]); break;
    }
}
/* game thread: apply the queued key presses */
static void panel_keys(void) {
    long keys = __sync_fetch_and_and(&gPanelKeys, 0); int i, c, ln, best, bd;
    if (!prow_selectable(gSel)) for (gSel = 0; gSel < NPROWS && !prow_selectable(gSel); gSel++) ;
    if (keys & PK_UP)   { for (i = gSel - 1; i >= 0; i--) if (prow_selectable(i) && prow_col(i) == prow_col(gSel)) { gSel = i; break; } }
    if (keys & PK_DOWN) { for (i = gSel + 1; i < NPROWS; i++) if (prow_selectable(i) && prow_col(i) == prow_col(gSel)) { gSel = i; break; } }
    if (keys & (PK_LEFT | PK_RIGHT)) {
        c = prow_col(gSel) + ((keys & PK_RIGHT) ? 1 : -1); ln = prow_line(gSel); best = -1; bd = 1000;
        for (i = 0; i < NPROWS; i++) if (prow_selectable(i) && prow_col(i) == c) {
            int dd = prow_line(i) - ln; if (dd < 0) dd = -dd;
            if (dd < bd) { bd = dd; best = i; }
        }
        if (best >= 0) gSel = best;
    }
    if (keys & PK_ENTER) panel_activate(gSel);
}
static void cv_color(u32 cv, int r, int g, int b) {
    u8 *c = (u8 *)(cv + offCvColor);
    c[offColR] = (u8)r; c[offColG] = (u8)g; c[offColB] = (u8)b; c[offColA] = 255;
}
static void cv_text(u32 cv, u32 font, float x, float y, const wchar *s) {
    *(float *)(cv + offCvCurX) = x; *(float *)(cv + offCvCurY) = y;
    gWPrintf((void *)cv, (void *)font, 0, L"%s", s);
}
/* game thread, from the HUD's PostRender */
static void panel_render(u32 cv) {
    u32 font, saveColor; float saveX, saveY, saveOX, saveOY, clipX, x0, y0, w, h, lh, colw; u8 saveStyle; u32 saveFont;
    int i, xl = 0, yl = 0, lines0 = 0, lines1 = 0, forced, on; static wchar t[200];
    if (gPanelReady <= 0 || !cv) return;
    panel_keys();
    saveFont = P(cv + offCvFont); saveColor = P(cv + offCvColor); saveStyle = *(u8 *)(cv + offCvStyle);
    saveX = *(float *)(cv + offCvCurX); saveY = *(float *)(cv + offCvCurY);
    saveOX = *(float *)(cv + offCvOrgX); saveOY = *(float *)(cv + offCvOrgY);
    font = offCvSmall > 0 && P(cv + offCvSmall) ? P(cv + offCvSmall) : saveFont;
    if (!font) return;
    *(u32 *)(cv + offCvFont) = font; *(u8 *)(cv + offCvStyle) = 1;   /* STY_Normal */
    *(float *)(cv + offCvOrgX) = 0; *(float *)(cv + offCvOrgY) = 0;
    gWStrLen((void *)cv, (void *)font, &xl, &yl, L"%s", L"Ag[X]");
    lh = (float)(yl > 4 ? yl : 14) + 2.0f;
    for (i = 0; i < NPROWS; i++) { if (PROWS[i].t == '|') { lines1 = 0; continue; } if (prow_col(i) == 0) lines0++; else lines1++; }
    clipX = *(float *)(cv + offCvClipX);
    colw = 330.0f; w = colw * 2 + 30; x0 = (clipX - w) / 2; if (x0 < 4) x0 = 4; y0 = 30;
    h = lh * ((lines0 > lines1 ? lines0 : lines1) + 4);
    if (gDrawTile && offWhiteTex > 0) {
        u32 level = cur_level(), tex = level ? P(level + offWhiteTex) : 0;
        if (tex) {
            fplane col = { 0.02f, 0.02f, 0.06f, 1.0f }, fog = { 0, 0, 0, 0 };
            float ul = offUSize > 0 ? (float)(int)P(tex + offUSize) : 1.0f, vl = offVSize > 0 ? (float)(int)P(tex + offVSize) : 1.0f;
            cv_color(cv, 8, 8, 16);
            gDrawTile((void *)cv, (void *)tex, x0, y0, w, h, 0, 0, ul, vl, 0, col, fog);
        }
    }
    cv_color(cv, 255, 220, 120);
    cv_text(cv, font, x0 + 10, y0 + 6, L"OPEN CASTLE DEBUG   Up/Down/Left/Right move   Enter: toggle   Del: close   (start) = forced on by the ini");
    for (i = 0; i < NPROWS; i++) {
        const struct prow *r = &PROWS[i]; float x, y;
        if (r->t == '|') continue;
        x = x0 + 10 + prow_col(i) * (colw + 20); y = y0 + 6 + lh * (prow_line(i) + 2);
        if (r->t == 'h') { cv_color(cv, 140, 180, 255); cv_text(cv, font, x, y, r->label); continue; }
        on = prow_state(r, &forced);
        if (r->t == 'a') wsprintfW(t, L"%s %s", i == gSel ? L">" : L" ", r->label);
        else if (r->t == 'x') wsprintfW(t, L"%s [ ] %s  (not wired yet)", i == gSel ? L">" : L" ", r->label);
        else wsprintfW(t, L"%s [%s] %s%s", i == gSel ? L">" : L" ", on > 0 ? L"X" : L" ", r->label, forced ? L"  (start)" : L"");
        if (i == gSel) cv_color(cv, 255, 255, 80);
        else if (r->t == 'x') cv_color(cv, 120, 120, 120);
        else cv_color(cv, 230, 230, 230);
        cv_text(cv, font, x, y, t);
    }
    *(u32 *)(cv + offCvFont) = saveFont; *(u32 *)(cv + offCvColor) = saveColor; *(u8 *)(cv + offCvStyle) = saveStyle;
    *(float *)(cv + offCvCurX) = saveX; *(float *)(cv + offCvCurY) = saveY;
    *(float *)(cv + offCvOrgX) = saveOX; *(float *)(cv + offCvOrgY) = saveOY;
}
/* game thread, ungated: pause / unpause with the panel */
static void panel_pause(void) {
    int want = gPanelPause; u32 pc = gPlayer ? P(gPlayer + offPlayerActor) : 0;
    gPanelPause = 0;
    if (N_SetPause < 0) N_SetPause = name_index(L"SetPause");
    if (pc) call_bool(pc, N_SetPause, want == 1);
}
/* game thread, tick job: the currency actions (KWPawn.AddToInventoryCollection on the carrier pawn) */
static void panel_currency(void) {
    static const wchar *const CLS[3] = { L"PumpkinPastyCollection", L"CauldronCakeCollection", L"JellybeanCollection" };
    static const int AMT[3] = { 10, 10, 100 }; static u32 cls[3]; static int N_AddColl = -1;
    struct { u32 cls; int n; u32 ret; } parms; int bits = gAddCurrency, i; u32 carrier = carrier_pawn(); void *fn; static wchar b[120];
    gAddCurrency = 0;
    if (N_AddColl < 0) N_AddColl = name_index(L"AddToInventoryCollection");
    if (!carrier || N_AddColl < 0 || !(fn = gFindFunction((void *)carrier, N_AddColl, 0))) { log_line(L"panel: AddToInventoryCollection not found"); return; }
    for (i = 0; i < 3; i++) if ((bits >> i) & 1) {
        if (!cls[i]) cls[i] = find_class(CLS[i]);
        if (!cls[i]) continue;
        parms.cls = cls[i]; parms.n = AMT[i]; parms.ret = 0;
        gPEOrig((void *)carrier, fn, &parms, 0);
        wsprintfW(b, L"panel: +%d %s", AMT[i], CLS[i]); log_line(b);
    }
}
