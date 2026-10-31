/*
 * HP3 Open Castle - test build (proof of concept). Ultimate ASI Loader plugin: plugins\HP3OpenCastle.asi
 *
 * What it does (see docs/OPEN_CASTLE_CHANGES.md for the full plan):
 *  1. Travel rewrite: hooks Engine.dll's FURL::FURL (through its incremental-link thunk), the one place every level
 *     change passes through (ChangeLevelTrigger, cutscene ChangeLevel, New Game). Each URL gets
 *     ?GameState=GState090, and the rules in rules.h reroute it: New Game -> InsideHub, classroom triggers -> final
 *     exams, story-map exits -> back to the hubs, dropped cutscene maps -> InsideHub.
 *  2. Door entrances: a worker thread watches the player's position; standing at one of the Open Castle doors queues
 *     a travel, which the ProcessEvent hook performs on the game thread by calling LevelInfo.ServerTravel.
 *  3. Logs everything to Documents\Harry Potter and the Prisoner of Azkaban\HP3_OpenCastle.log.
 *
 *  4. Return points: when the player arrives back on a hub, the ProcessEvent hook catches the hero controller's
 *     TravelPostAccept. The game places the player at the SmartStart whose PreviousLevelName matches the map they
 *     came from; story maps rerouted by Open Castle have none, so the player would land in the GState holding room.
 *     We lend the pawn's PreviousLevelName a nearby SmartStart's name for that one call (so Ron and Hermione follow
 *     too), then optionally RelocatePawn to the Open Castle door.
 *  5. Unlocks (unlocks.h): spells (WeaponWand.HasSpell made native), doors, the Marauder's Map and the shortcut
 *     passwords. Start state from the ini, found state in the save (PersistentStore OC_* keys).
 *  6. Leftover goodies (leftovers.h): items a chest threw out but the player didn't pick up come back on the next
 *     visit.
 *  7. Softlock escape (hubmenu.h): RETURN TO HUB in the pause menu's exit dialog.
 *  8. Door gates: a door's loading trigger only works while the door is unlocked, and its portcullis (a TriggerToggle
 *     KWMover, the one the vanilla cutscenes trigger) is raised or lowered to match. Which doors are unlocked comes
 *     from Documents\Harry Potter and the Prisoner of Azkaban\HP3_OpenCastle.ini, [Doors] <key>=1/0 (default 1), read
 *     every 2 s, so it can be changed while the game runs. The randomizer will drive the same switch.
 *  9. Shop prices (shop.h): [Shop] Cheap=1 makes wizard cards and portrait passwords cost 50% (rounded up).
 *
 * Not in this build: item gating (every door is open), spell hooks, the Marauder's Map pickup.
 *
 * Hooks are only installed if the bytes at each site are exactly what was verified on the retail build; otherwise
 * the log says so and the game runs unmodified.
 * No CRT; built with clang + lld-link (see build.sh).
 */
typedef unsigned short wchar;
typedef unsigned int   u32;
typedef unsigned char  u8;
typedef int            BOOL;
typedef void*          HANDLE;
typedef void*          HWND;
#define WINAPI __stdcall
#define IMP __declspec(dllimport)

typedef struct { unsigned short wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMs; } SYSTEMTIME;
IMP HANDLE WINAPI GetModuleHandleW(const wchar*);
IMP void*  WINAPI GetProcAddress(HANDLE, const char*);
IMP HANDLE WINAPI CreateThread(void*, u32, u32 (WINAPI*)(void*), void*, u32, u32*);
IMP void   WINAPI Sleep(u32);
IMP BOOL   WINAPI ReadProcessMemory(HANDLE, const void*, void*, u32, u32*);
IMP HANDLE WINAPI GetCurrentProcess(void);
IMP BOOL   WINAPI DisableThreadLibraryCalls(HANDLE);
IMP HANDLE WINAPI CreateFileW(const wchar*, u32, u32, void*, u32, u32, HANDLE);
IMP BOOL   WINAPI WriteFile(HANDLE, const void*, u32, u32*, void*);
IMP BOOL   WINAPI CloseHandle(HANDLE);
IMP void   WINAPI GetLocalTime(SYSTEMTIME*);
IMP int    WINAPI WideCharToMultiByte(u32, u32, const wchar*, int, char*, int, const char*, BOOL*);
IMP BOOL   WINAPI VirtualProtect(void*, u32, u32, u32*);
IMP void*  WINAPI VirtualAlloc(void*, u32, u32, u32);
IMP BOOL   WINAPI FlushInstructionCache(HANDLE, const void*, u32);
IMP int    __cdecl wsprintfW(wchar*, const wchar*, ...);
IMP int    __cdecl wsprintfA(char*, const char*, ...);
IMP long   WINAPI SHGetFolderPathW(HWND, int, HANDLE, u32, wchar*);
IMP u32    WINAPI GetPrivateProfileIntW(const wchar*, const wchar*, int, const wchar*);
IMP u32    WINAPI GetTickCount(void);
IMP BOOL   WINAPI ReadFile(HANDLE, void*, u32, u32*, void*);
IMP u32    WINAPI GetFileSize(HANDLE, u32*);
IMP u32    WINAPI SetFilePointer(HANDLE, int, int*, u32);
IMP short  WINAPI GetAsyncKeyState(int);
IMP HWND   WINAPI GetForegroundWindow(void);
IMP u32    WINAPI GetWindowThreadProcessId(HWND, u32*);
IMP u32    WINAPI GetCurrentProcessId(void);
IMP BOOL   WINAPI WritePrivateProfileStringW(const wchar*, const wchar*, const wchar*, const wchar*);

#define OC_VERSION L"poc9r"
/* OC_DEBUG 1 (default, build/HP3OpenCastle.asi): the development build, with the debug panel (Delete), the item test
   grid (backslash) and noclip (F6). OC_DEBUG 0 (build/release/HP3OpenCastle.asi, build.sh): the release build, with
   no key thread at all, so none of those can be opened; everything else is the same code. */
#ifndef OC_DEBUG
#define OC_DEBUG 1
#endif
#if OC_DEBUG
#define OC_BUILD L"debug"
#else
#define OC_BUILD L"release"
#endif

int _fltused = 0;
void *memset(void *d, int c, unsigned n) { volatile u8 *p = (volatile u8 *)d; while (n--) *p++ = (u8)c; return d; }
void *memcpy(void *d, const void *s, unsigned n) { volatile u8 *p = (volatile u8 *)d; const u8 *q = s; while (n--) *p++ = *q++; return d; }

static HANDLE gProc;
static u32 *gObjs, *gNames;
#include "../common/ue2read.h"
#include "rules.h"

/* ---- log (called from the game thread and the worker; a tiny spinlock keeps lines whole) ---- */
static wchar gLogPath[600];
static volatile long gLogLock;
static void log_line(const wchar *w) {
    static char u8[1600]; static wchar line[800]; u32 wr; SYSTEMTIME t; int n; HANDLE f;
    while (__sync_lock_test_and_set(&gLogLock, 1)) Sleep(0);
    GetLocalTime(&t);
    wsprintfW(line, L"%04d-%02d-%02d %02d:%02d:%02d  %s\r\n", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, w);
    n = WideCharToMultiByte(65001, 0, line, -1, u8, sizeof u8, 0, 0);
    f = CreateFileW(gLogPath, 0x0004 /*FILE_APPEND_DATA*/, 1 | 2, 0, 4 /*OPEN_ALWAYS*/, 0x80, 0);
    if (f != (HANDLE)-1) { if (n > 1) WriteFile(f, u8, n - 1, &wr, 0); CloseHandle(f); }
    __sync_lock_release(&gLogLock);
}

/* ---- shared state (worker writes, game thread reads) ---- */
static wchar gMapBuf[2][64];
static const wchar *volatile gCurMap = L"";   /* map the player is on, as a file name */
static volatile u32 gPlayer;                  /* Player (viewport) object; persists across levels */
static int offPlayerActor = -1, offPawn = -1, offLoc = -1, offLevel = -1, offID = -1;
static int N_ServerTravel = -1, N_TravelPostAccept = -1, N_RelocatePawn = -1, N_EnableShortcuts = -1;
static int offPrevLevel = -1, offGame = -1, offPPM = -1;
static volatile u32 gClsHero;                 /* KWHeroController class */
static volatile int gPendingShortcuts;
static volatile int gPending; static wchar gPendingURL[128];
static volatile int gHubReturn;               /* the next arrival on InsideHub came from RETURN TO HUB */
static volatile int gNewGame;                 /* a New Game travel was seen: forget the previous game's unlocks */

/* ---- hook 1: FURL::FURL(FURL* Base, const TCHAR* TextURL, ETravelType) ---- */
typedef void *(__thiscall *furl_fn)(void *self, void *base, const wchar *text, int type);
static furl_fn gFurlOrig;
static wchar gUrlOut[600];
static const wchar *locked_challenge(const wchar *url);   /* after DOORS */
static void *__thiscall furl_hook(void *self, void *base, const wchar *text, int type) {
    const wchar *use = text, *lk; wchar why[128]; static wchar msg[900], blocked[600];
    /* where the travel really goes (after Open Castle's own rewrite): a locked challenge or exam is refused, from any
       trigger (the chamber copies in the B maps too) */
    if (text && (lk = locked_challenge(oc_rewrite((const wchar *)gCurMap, text, gUrlOut, 600, why, 128) ? gUrlOut : text)) != 0) {
        wcpy(blocked, L"HP3_InsideHub?GameState=GState090", 600);
        wsprintfW(msg, L"TRAVEL  from %s:  %s  ->  %s   [%s is locked]", gCurMap[0] ? gCurMap : L"?", text, blocked, lk);
        log_line(msg);
        return gFurlOrig(self, base, blocked, type);
    }
    if (text && oc_rewrite((const wchar *)gCurMap, text, gUrlOut, 600, why, 128)) {
        use = gUrlOut;
        if (wieq(why, L"new game starts in Hogwarts")) gNewGame = 1;
        wsprintfW(msg, L"TRAVEL  from %s:  %s  ->  %s   [%s]", gCurMap[0] ? gCurMap : L"?", text, gUrlOut, why);
        log_line(msg);
    } else if (text && text[0] && !contains_ci(text, L"127.0.0.1")) {
        wsprintfW(msg, L"travel  from %s:  %s   (unchanged)", gCurMap[0] ? gCurMap : L"?", text);
        log_line(msg);
    }
    return gFurlOrig(self, base, use, type);
}

/* ---- hook 2: UObject::ProcessEvent, used only to run a queued door travel on the game thread ---- */
typedef void (__thiscall *pe_fn)(void *self, void *func, void *parms, void *result);
typedef void *(__thiscall *ff_fn)(void *self, int fname, int global);
static pe_fn gPEOrig; static ff_fn gFindFunction;
static volatile int gInTravel;
static void do_travel(void) {
    struct { wchar *data; int num, max; u32 bItems; } parms;
    u32 pc, level; void *fn; static wchar msg[300];
    if (!gPlayer || offPlayerActor < 0 || offLevel < 0 || N_ServerTravel < 0) return;
    pc = rd32(gPlayer + offPlayerActor);
    level = pc ? rd32(pc + offLevel) : 0;
    if (!level) { log_line(L"door travel skipped: no level"); return; }
    fn = gFindFunction((void *)level, N_ServerTravel, 0);
    if (!fn) { log_line(L"door travel skipped: LevelInfo.ServerTravel not found"); return; }
    parms.data = gPendingURL; parms.num = wlen(gPendingURL) + 1; parms.max = parms.num; parms.bItems = 1;
    wsprintfW(msg, L"DOOR  %s: ServerTravel(%s)", gCurMap, gPendingURL);
    log_line(msg);
    gPEOrig((void *)level, fn, &parms, 0);
}
/* ---- return points: arriving on `hub` from `from` ---- */
struct ret { const wchar *hub, *from, *smart; int move; float x, y, z; };
static const struct ret RETS[] = {
    /* hub              came from                use this SmartStart        move?  door position */
    { L"HP3_InsideHub",  L"HP3_Adv6Paddock",     L"HP3_GroundsHub",           0,    0,     0,    0 },   /* entrance hall */
    { L"HP3_InsideHub",  L"HP3_Adv1Express",     L"HP3_PortraitPassword_D",   1, 1427,  -960, 2154 },
    { L"HP3_InsideHub",  L"HP3_Adv2ExpPatronum", L"HP3_PortraitPassword_C",   1,  838, -1844, 1194 },
    { L"HP3_InsideHub",  L"HP3_Adv3Library",     L"HP3_PortraitPassword_C",   1, -710, -1349,  778 },
    /* exams: back in front of the classroom door (where its voice-line PopupTrigger stood, clear of the loading trigger) */
    { L"HP3_InsideHub",  L"HP3_FA1Ron",          L"HP3_GroundsHub",           1,  418, -2221, 1206 },   /* DADA */
    { L"HP3_InsideHub",  L"HP3_FA2Hermione",     L"HP3_GroundsHub",           1, -3309,  2200,  533 },   /* Transfiguration (ToDrac door) */
    { L"HP3_InsideHub",  L"HP3_FA3Harry",        L"HP3_GroundsHub",           1, -1365, -2829,  948 },   /* Charms */
    { L"HP3_DungeonHUB", L"HP3_Adv5Shack",       0,                           1, -481,  2805, -341 },   /* no SmartStarts */
    { L"HP3_InsideHub",  L"HP3_BeanBonus",       L"HP3_GroundsHub",           1,  733,   392,   74 },   /* entrance hall, south door */
};
#define NRETS (int)(sizeof RETS / sizeof RETS[0])

static const wchar *level_map(u32 level) {   /* game thread */
    static wchar id[32];
    if (!level || offID < 0 || !read_fstring(level + offID, id, 32)) return 0;
    return map_for_id(id);
}
static void call_relocate(u32 pawn, const struct ret *r) {
    struct { float x, y, z; u32 ret; } parms; void *fn; static wchar msg[200];
    if (N_RelocatePawn < 0 || !(fn = gFindFunction((void *)pawn, N_RelocatePawn, 0))) { log_line(L"return: RelocatePawn not found"); return; }
    parms.x = r->x; parms.y = r->y; parms.z = r->z; parms.ret = 0;
    gPEOrig((void *)pawn, fn, &parms, 0);
    wsprintfW(msg, L"return: moved to the door (%d, %d, %d): %s", (int)r->x, (int)r->y, (int)r->z, parms.ret ? L"ok" : L"FAILED");
    log_line(msg);
}
/* the hero controller's TravelPostAccept: choose where the player appears */
static void travel_post_accept(void *self, void *func, void *parms, void *result) {
    u32 pc = (u32)self, pawn, sv[3]; const wchar *hub; wchar prev[64]; static wchar msg[300], lend[64];
    const struct ret *r = 0; int i;
    pawn = offPawn >= 0 ? rd32(pc + offPawn) : 0;
    hub = level_map(offLevel >= 0 ? rd32(pc + offLevel) : 0);
    if (!pawn || !hub || offPrevLevel < 0x28 || !rd(pawn + offPrevLevel, sv, 12)) { gPEOrig(self, func, parms, result); return; }
    if (!read_fstring(pawn + offPrevLevel, prev, 64)) prev[0] = 0;
    for (i = 0; i < NRETS; i++) if (wieq(RETS[i].hub, hub) && wieq(RETS[i].from, prev)) { r = &RETS[i]; break; }
    if (gHubReturn) {   /* RETURN TO HUB: the entrance hall, whatever map it came from */
        static const struct ret hubret = { L"HP3_InsideHub", L"", L"HP3_GroundsHub", 0, 0, 0, 0 };
        gHubReturn = 0;
        if (wieq(hub, L"HP3_InsideHub")) r = &hubret;
    }
    wsprintfW(msg, L"arrive %s from \"%s\"%s", hub, prev, r ? L" -> Open Castle return point" : L"");
    log_line(msg);
    if (r && r->smart) {   /* lend the SmartStart's name for this one call, then put the original string back */
        u32 tmp[3];
        wcpy(lend, r->smart, 64);
        tmp[0] = (u32)lend; tmp[1] = wlen(lend) + 1; tmp[2] = tmp[1];
        memcpy((void *)(pawn + offPrevLevel), tmp, 12);
        gPEOrig(self, func, parms, result);
        if (rd32(pawn + offPrevLevel) == (u32)lend) memcpy((void *)(pawn + offPrevLevel), sv, 12);
        else log_line(L"return: PreviousLevelName was replaced during TravelPostAccept (left as is)");
    } else gPEOrig(self, func, parms, result);
    if (r && r->move) call_relocate(rd32(pc + offPawn), r);
}
static void enable_shortcuts(void) {
    u32 pc, level, game, ppm; void *fn;
    if (!gPlayer || offGame < 0 || offPPM < 0 || N_EnableShortcuts < 0) return;
    pc = rd32(gPlayer + offPlayerActor); level = pc ? rd32(pc + offLevel) : 0;
    game = level ? rd32(level + offGame) : 0; ppm = game ? rd32(game + offPPM) : 0;
    if (!ppm || !(fn = gFindFunction((void *)ppm, N_EnableShortcuts, 0))) { log_line(L"shortcuts: no PortraitPasswordManager on this map"); return; }
    gPEOrig((void *)ppm, fn, 0, 0);
    log_line(L"shortcuts: all shortcut passwords unlocked");
}
/* ---- door gates (game thread part): Trigger the queued gate movers ---- */
#define MAXGATES 16
#define GATEMOVERS 4   /* movers sharing one gate tag (MBOMDoor: a portcullis and two doors) */
static volatile u32 gGateObj[MAXGATES][GATEMOVERS]; static volatile u32 gGateFire;   /* bit i: Trigger gate i's movers */
static int N_Trigger = -1;
static void fire_gates(void) {
    struct { u32 other, instigator; } parms; void *fn; int i; u32 bits = gGateFire;
    gGateFire = 0;
    for (i = 0; i < MAXGATES; i++) if ((bits >> i) & 1) {
        int k;
        for (k = 0; k < GATEMOVERS; k++) {
            u32 m = gGateObj[i][k];
            if (!m || N_Trigger < 0 || !(fn = gFindFunction((void *)m, N_Trigger, 0))) continue;   /* state-aware: TriggerToggle.Trigger */
            parms.other = parms.instigator = 0;
            gPEOrig((void *)m, fn, &parms, 0);
        }
    }
}

#include "leftovers.h"
#include "hubmenu.h"
static volatile int gGridPending; static void grid_spawn(void); static void items_apply(void);
static volatile u32 gGridTake; static void grid_take(void);
static volatile u32 gBlockSpawn, gBlockKill; static void blockers_apply(void); static void trig_apply(void);
static volatile u32 gTrigFire; static volatile int gSilFire;
static volatile u32 gChShow, gChFire, gChTrigSet; static void chamber_apply(void);   /* chamber.h */
static void ap_tick(void); static void ap_event(u32 self, int fname, int linkedBefore);   /* ap.h */
static int N_BeginState, N_Timer, N_VendorOnDraw; static int apshop_begin_state(void *self, void *func, void *parms, void *result);
static int apshop_timer(u32 o); static void apshop_dialog(u32 page);   /* apshop.h */
static int N_Touch; static int ap_touch(void *self, void *func, void *parms);   /* apshop.h */
static int ap_pdr(void *self, void *func, void *parms, void *result);   /* apshop.h */
static volatile int gApCheap = -1;   /* ap.h: the room's shop_prices (-1: no room, the ini decides) */
/* voice lines switched off for good (touch PopupTriggers, collision off on each map): the exam doors' "that's the
   door to the ... spell challenge" comments */
struct silence { const wchar *map, *actor, *why; };
static const struct silence SILENCE[] = {
    { L"HP3_InsideHub", L"PopupTrigger10", L"DADA exam door (PC_HRY_CHALCH1COMMENTS_6)" },
    { L"HP3_InsideHub", L"PopupTrigger17", L"Transfiguration exam door (PC_HRY_DOORCOMMENTS_34)" },
    { L"HP3_InsideHub", L"PopupTrigger13", L"Charms exam door (PC_RON_DOORCOMMENTS_40)" },
};
#define NSILENCE (int)(sizeof SILENCE / sizeof SILENCE[0])
static volatile u32 gSilObj[NSILENCE]; static volatile int gSilFire;
static int N_PlayerTick = -1, N_Tick = -1;
static volatile int gPanelPause, gAddCurrency, gPanelOpen; static void panel_pause(void); static void panel_currency(void);
static int N_PostRender; static u32 gClsHUD; static void panel_render(u32 cv);
static volatile int gNoclip; static int gNoclipOn; static void noclip_tick(u32 pc, float dt);
static void __thiscall pe_hook(void *self, void *func, void *parms, void *result) {
    int fname = func ? *(int *)((u8 *)func + 0x20) : -1, safe;
    /* Jobs that spawn, destroy or change actors run only from a tick event (PlayerTick / Tick, called while the
       level ticks). ProcessEvent is also called while the frame is drawn (HUD and canvas events from
       UGameEngine::Draw), and destroying an actor there crashed in RenderLevel (poc7c, a grid pickup). */
    safe = fname >= 0 && (fname == N_PlayerTick || fname == N_Tick);
    if (gPending && !gInTravel) { gInTravel = 1; gPending = 0; do_travel(); gInTravel = 0; }
    if (gMenuPatch && !gInTravel) { gInTravel = 1; gMenuPatch = 0; hub_patch(); gInTravel = 0; }
    if (gPanelPause && !gInTravel) { gInTravel = 1; panel_pause(); gInTravel = 0; }
    if (safe && !gInTravel) {
        gInTravel = 1;
        if (gPendingShortcuts) { gPendingShortcuts = 0; items_apply(); }
        if (gGridTake) grid_take();
        if (gPendingRespawn) { gPendingRespawn = 0; lo_respawn(); }
        if (gGateFire) fire_gates();
        if (gGridPending) { grid_spawn(); gGridPending = 0; }
        if (gBlockSpawn || gBlockKill) blockers_apply();
        if (gAddCurrency) panel_currency();
        if (gTrigFire || gSilFire) trig_apply();
        ap_tick();   /* Archipelago: items in, vanilla rewards taken back, the save side */
        if (gChShow || gChFire || gChTrigSet) chamber_apply();
        gInTravel = 0;
    }
    if (!func || !self) { gPEOrig(self, func, parms, result); return; }
    if (gPanelOpen && fname == N_PostRender && N_PostRender >= 0 && gClsHUD && !gInTravel && is_a_fast((u32)self, gClsHUD)) {
        gPEOrig(self, func, parms, result);   /* the HUD first, then the debug panel on top */
        gInTravel = 1; panel_render(parms ? *(u32 *)parms : 0); gInTravel = 0;
        return;
    }
    if ((gNoclip || gNoclipOn) && fname == N_PlayerTick && !gInTravel && gPlayer && offPlayerActor >= 0
        && (u32)self == *(u32 *)(gPlayer + offPlayerActor)) {   /* noclip: after the controller's own PlayerTick, once a frame */
        gPEOrig(self, func, parms, result);
        gInTravel = 1; noclip_tick((u32)self, parms ? *(float *)parms : 0); gInTravel = 0;
        return;
    }
    if (fname == N_MyOnClick && N_MyOnClick >= 0 && !gInTravel) {
        int done; gInTravel = 1; done = hub_click((u32)self, (u32 *)parms); gInTravel = 0;
        if (done) return;
    }
    if (fname == N_PostBeginPlay || fname == N_Destroyed || fname == N_PostPDR || fname == N_TravelExporting) {
        int linked = gNPu;
        lo_event((u32)self, fname);
        if (fname == N_PostBeginPlay || fname == N_Destroyed) ap_event((u32)self, fname, linked);   /* also for our own spawns */
    }
    if (fname == N_TravelPostAccept && N_TravelPostAccept >= 0 && gClsHero
        && is_a_cls(*(u32 *)((u8 *)self + 0x24), gClsHero)) { travel_post_accept(self, func, parms, result); return; }
    /* shopsanity (apshop.h): sales start / end, stands that refill themselves, the sale dialog */
    if (fname == N_BeginState && N_BeginState >= 0 && !gInTravel && apshop_begin_state(self, func, parms, result)) return;
    if (fname == N_Timer && N_Timer >= 0 && !gInTravel && apshop_timer((u32)self)) return;
    if (fname == N_Touch && N_Touch >= 0 && !gInTravel && ap_touch(self, func, parms)) return;   /* a location's pickup */
    if (fname == N_VendorOnDraw && N_VendorOnDraw >= 0 && !gInTravel) apshop_dialog((u32)self);
    if (fname == N_PostPDR && N_PostPDR >= 0 && !gInTravel && ap_pdr(self, func, parms, result)) return;   /* card locations */
    gPEOrig(self, func, parms, result);
}

/* ---- patching ---- */
static int patch_thunk(u8 *thunk, void *hook, void **orig) {
    u32 old; int rel;
    if (!thunk || thunk[0] != 0xE9) return 0;
    rel = *(int *)(thunk + 1);
    *orig = thunk + 5 + rel;
    if (!VirtualProtect(thunk, 5, 0x40 /*PAGE_EXECUTE_READWRITE*/, &old)) return 0;
    *(int *)(thunk + 1) = (int)((u8 *)hook - (thunk + 5));
    VirtualProtect(thunk, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), thunk, 5);
    return 1;
}
static int patch_detour5(u8 *fn, const u8 *expect, void *hook, void **tramp) {
    u32 old; u8 *t; int i;
    if (!fn) return 0;
    for (i = 0; i < 5; i++) if (fn[i] != expect[i]) return 0;
    t = VirtualAlloc(0, 16, 0x3000 /*MEM_COMMIT|MEM_RESERVE*/, 0x40);
    if (!t) return 0;
    for (i = 0; i < 5; i++) t[i] = fn[i];
    t[5] = 0xE9; *(int *)(t + 6) = (int)((fn + 5) - (t + 10));
    *tramp = t;
    if (!VirtualProtect(fn, 5, 0x40, &old)) return 0;
    fn[0] = 0xE9; *(int *)(fn + 1) = (int)((u8 *)hook - (fn + 5));
    VirtualProtect(fn, 5, old, &old);
    FlushInstructionCache(GetCurrentProcess(), fn, 5);
    return 1;
}
static int gHookFurl, gHookPE;
static void install_hooks(void) {
    static const u8 pe_prologue[5] = { 0x55, 0x8B, 0xEC, 0x6A, 0xFF };
    HANDLE eng = GetModuleHandleW(L"Engine.dll"), core = GetModuleHandleW(L"Core.dll");
    if (eng) gHookFurl = patch_thunk(GetProcAddress(eng, "??0FURL@@QAE@PAV0@PBGW4ETravelType@@@Z"), (void *)furl_hook, (void **)&gFurlOrig);
    if (core) {
        gFindFunction = (ff_fn)GetProcAddress(core, "?FindFunction@UObject@@QAEPAVUFunction@@VFName@@H@Z");
        if (gFindFunction)
            gHookPE = patch_detour5(GetProcAddress(core, "?ProcessEvent@UObject@@UAEXPAVUFunction@@PAX1@Z"), pe_prologue,
                                    (void *)pe_hook, (void **)&gPEOrig);
    }
}

/* ---- doors: stand here to enter (player positions marked in the playthrough) ---- */
struct door { const wchar *map; float x, y, z; const wchar *dest; const wchar *name; const wchar *key; const wchar *gate; const wchar *bar;
               int inv; const wchar *trig[2]; };
/* inv: the portcullis is open at key 0 (down at key 1), the reverse of DADA / Library.
   trig: the map's own loading triggers for this door, switched off (no collision) while it's locked. */
static const struct door DOORS[] = {
    /* map               stand here            goes to                 name                        ini key    portcullis mover tag      lock bar (StaticMeshActor, DoorLockBar_SM) */
    { L"HP3_InsideHub",  731, 1615,   74, L"HP3_Adv6Paddock",     L"entrance hall north door", L"Paddock", 0,                        L"StaticMeshActor42", 0, { 0, 0 } },
    { L"HP3_InsideHub", 1427, -960, 2154, L"HP3_Adv1Express",     L"7th floor east door",      L"Express", 0,                        L"StaticMeshActor228", 0, { 0, 0 } },
    { L"HP3_InsideHub",  838, -1844, 1194, L"HP3_Adv2ExpPatronum", L"DADA portcullis",         L"DADA",    L"DADA_PortToExpecto_Mvr", 0, 0, { 0, 0 } },
    { L"HP3_InsideHub", -710, -1349,  778, L"HP3_Adv3Library",     L"Library portcullis",      L"Library", L"Library_Port_Mvr",       0, 0, { 0, 0 } },
    { L"HP3_DungeonHUB", -481, 2805, -341, L"HP3_Adv5Shack",       L"dungeon door 1",          L"Shack",   0,                        L"StaticMeshActor315", 0, { 0, 0 } },
    /* the Wizard Card bonus room (vanilla: only after the Harry Potter card, see SUPPRESS) */
    { L"HP3_InsideHub",  733,  392,   74, L"HP3_BeanBonus",       L"entrance hall south door", L"BonusRoom", 0,                      L"StaticMeshActor50", 0, { 0, 0 } },
    /* challenges: no loading trigger of ours (dest 0). The Monster Book room has a door (portcullis + two doors, all
       tag MBOMDoor, movable: 7200TaskedCharms opens them); the hippogriff and doxy challenges have no door, so their
       start triggers are switched off while locked (EVENTGATES). */
    { L"HP3_InsideHub",     0,     0,    0, 0,                     L"Monster Book challenge",  L"MonsterBook", L"MBOMDoor",          0, 0, { 0, 0 } },
    { L"HP3_GroundsHub",    0,     0,    0, 0,                     L"hippogriff challenge",    L"Hippogriff",  0,                    0, 0, { 0, 0 } },
    { L"HP3_GroundsHub",    0,     0,    0, 0,                     L"doxy challenge",          L"Doxy",        0,                    0, 0, { 0, 0 } },
    /* the side room (challenge chamber): the castle's own portcullises (KWMover31/32/33, bPersistent, key 0 = up at
       z 392, key 1 = down, 136 lower) in front of the vanilla loading triggers. Carpe's starts up in GState090,
       Draconifors' and Glacius' down. */
    { L"HP3_InsideHub",     0,     0,    0, 0,                     L"Carpe Retractum challenge", L"ChCarpe",   L"ChalChamb_CR_Port",   0, 1, { L"ChangeLevelTrigger9", 0 } },
    { L"HP3_InsideHub",     0,     0,    0, 0,                     L"Draconifors challenge",     L"ChDracLap", L"ChalChamb_Drac_Port", 0, 1, { L"ChangeLevelTrigger10", 0 } },
    { L"HP3_InsideHub",     0,     0,    0, 0,                     L"Glacius challenge",         L"ChGlacius", L"ChalChamb_Gla_Port",  0, 1, { L"ChangeLevelTrigger11", 0 } },
    /* the final exams, through the classroom doors (portcullis key 1 = up, like DADA / Library). Their loading
       triggers are the vanilla ones, retargeted by rules.h: DADA's touch Trigger58 fires ChangeLevelTrigger3
       (HP3_Ch1CarpeRetractum -> FA1); Charms' ChangeLevelTrigger6 (HP3_Ch3Glacius -> FA3); Transfiguration's door
       is the "ToDrac" door on the floor below the classroom, ChangeLevelTrigger7 (HP3_Ch2Draconifors.unr -> FA2,
       by the player's position, see gAtTransfigDoor). In GState090 Ron's portcullis starts up, the other two down. */
    { L"HP3_InsideHub",     0,     0,    0, 0,                     L"Ron's exam (DADA)",          L"ExamRon",      L"DADA_PortToCarpe_Mvr", 0, 0, { L"Trigger58", 0 } },
    { L"HP3_InsideHub",     0,     0,    0, 0,                     L"Hermione's exam (Transfiguration)", L"ExamHermione", L"ToDrac_Prt_Mvr", 0, 0, { L"ChangeLevelTrigger7", 0 } },
    { L"HP3_InsideHub",     0,     0,    0, 0,                     L"Harry's exam (Charms)",      L"ExamHarry",    L"Glacius_Port_Mvr",     0, 0, { L"ChangeLevelTrigger6", 0 } },
};
#define NDOORS (int)(sizeof DOORS / sizeof DOORS[0])
static int gDoorOn[16];
/* the challenge a travel URL goes to, if its door is locked (0 otherwise). Map token = up to '?' or '.' */
static const wchar *locked_challenge(const wchar *url) {
    static const wchar *const CH[6][2] = { { L"HP3_Ch1CarpeRetractum", L"ChCarpe" }, { L"HP3_Ch2Draconifors", L"ChDracLap" },
                                           { L"HP3_Ch3Glacius", L"ChGlacius" }, { L"HP3_FA1Ron", L"ExamRon" },
                                           { L"HP3_FA2Hermione", L"ExamHermione" }, { L"HP3_FA3Harry", L"ExamHarry" } };
    wchar tok[64]; int n = 0, c, d;
    while (url[n] && url[n] != '?' && url[n] != '.' && n < 63) { tok[n] = url[n]; n++; }
    tok[n] = 0;
    for (c = 0; c < 6; c++) if (wieq(tok, CH[c][0]))
        for (d = 0; d < NDOORS; d++) if (weq(DOORS[d].key, CH[c][1])) return gDoorOn[d] ? 0 : DOORS[d].name;
    return 0;
}
#define DOOR_R   90.0f    /* horizontal reach */
#define DOOR_DZ 160.0f
#define ARM_R   300.0f    /* must have been this far from every door on the map before a door works */

/* ---- door gates (worker part) ---- */
static wchar gIniPath[600];
static volatile int gSaveDoor[16];   /* door found in this save (unlocks.h) */
static volatile int gIniDoor[16];    /* door's start state (ini), for the debug panel */
static int offTag = -1, offKeyNum = -1;
static u32 gClsMover;
static u32 gGateLevel; static u32 gGateLast[MAXGATES];   /* GetTickCount of our last Trigger, per gate */
/* cutscene triggers Open Castle switches off (their Event is set to None once the map loads; the trigger stays but
   fires nothing). Only if the Event is the expected one, so a different map version is left alone. */
/* key 0: always off. key set: the trigger works only while that door / challenge is unlocked. */
struct suppress { const wchar *map, *actor, *event, *why, *key; int tag; };   /* tag: switch the actor's Tag, not its Event */
static const struct suppress SUPPRESS[] = {
    /* 3rd floor corridor in front of DADA, GState090 group: 9240RonExamIntroPt1 = ChangeLevel HP3_FA1Ron + GState100 */
    { L"HP3_InsideHub",  L"CutSceneTrigger13",      L"9240RonExamIntroPt1",       L"Ron's exam intro on the 3rd floor (I1)", 0, 0 },
    /* GroundsHub walk-in to the COMC course selection (CutSceneFunctionality group, every state) */
    { L"HP3_GroundsHub", L"CutSceneTrigger15",      L"ChangeLevel_COMC_MINIGAME", L"hippogriff challenge entrance",          L"Hippogriff", 0 },
    /* the well: a companion spell starts the next doxy round (RoundRobin PixieBattleRoundRobin) */
    { L"HP3_GroundsHub", L"CompanionSpellTrigger10", L"PixieBattleRoundRobin",    L"doxy challenge start",                   L"Doxy", 0 },
    /* the Monster Book cauldron: a companion spell starts the next round (RoundRobin MBRobin); backs up the door */
    { L"HP3_InsideHub",  L"CompanionSpellTrigger3",  L"MBRobin",                  L"Monster Book challenge start",           L"MonsterBook", 0 },
    /* the Harry Potter card (74 cards) sends the player to the Wizard Card bonus room: VI_FinalWizardCard0 GotCardEvent
       -> Dispatcher10 -> after 3 s WCBonusRoom_Trig = this ChangeLevelTrigger14. Open Castle reaches the room by the
       entrance hall south door instead, so the trigger loses its Tag and nothing can fire it. */
    { L"HP3_InsideHub",  L"ChangeLevelTrigger14",    L"WCBonusRoom_Trig",         L"Harry Potter card -> bonus room teleport", 0, 1 },
    /* Glacius entrance door (KWMover2, TriggerToggle). The first-visit intro 7400GlaciusSal (GState070 only) opened it
       once and this touch trigger, on the hallway side, shut it once you were in. In GState090 the replay intro
       7301SP3GlacIntroAlt opens it, puts Harry inside and shuts it itself, so this trigger only re-opened it (the
       companions walk into it): seen in play as "shut, then opened again" (poc8p). */
    { L"HP3_Ch3Glacius", L"Trigger24",               L"Glacius_RmA_Enter_Door",   L"Glacius entrance door re-open (touch)",   0, 0 },
};
#define NSUPPRESS (int)(sizeof SUPPRESS / sizeof SUPPRESS[0])
static int offEvent = -1;
static u32 gSuppObj[NSUPPRESS]; static int gSuppEvent[NSUPPRESS];   /* the trigger and its original Event name */

/* lock bars: separate StaticMeshActors (DoorLockBar_SM) in front of the doors without a portcullis. Static actors
   can't be moved, so an unlocked door's bar is hidden: DrawType DT_StaticMesh (8) -> DT_None (0). */
static int offDrawType = -1;
static u32 gBarObj[MAXGATES];
/* the doors' own loading triggers (DOORS.trig): collision follows the door; switched on the game thread (trig_apply) */
static volatile u32 gTrigObj[MAXGATES][2]; static int gTrigApplied[MAXGATES]; static volatile int gTrigWant[MAXGATES];
static volatile u32 gTrigFire;
struct findgate { u32 level; int names[MAXGATES]; u32 found[MAXGATES][GATEMOVERS]; int nfound[MAXGATES]; int bars[MAXGATES]; u32 barfound[MAXGATES]; int supp[NSUPPRESS];
                 int sil[NSILENCE];
                 int trigs[MAXGATES][2]; u32 trigfound[MAXGATES][2]; };
static int v_findgate(u32 o, int name, u32 cls, void *c) {
    struct findgate *f = c; int i, tag;
    if (!cls || rd32(o + offLevel) != f->level) return 0;
    for (i = 0; i < NDOORS && i < MAXGATES; i++) {
        if (f->bars[i] >= 0 && name == f->bars[i]) f->barfound[i] = o;
        if (f->trigs[i][0] >= 0 && name == f->trigs[i][0]) f->trigfound[i][0] = o;
        if (f->trigs[i][1] >= 0 && name == f->trigs[i][1]) f->trigfound[i][1] = o;
    }
    for (i = 0; i < NSUPPRESS; i++) if (f->supp[i] >= 0 && name == f->supp[i]) gSuppObj[i] = o;
    for (i = 0; i < NSILENCE; i++) if (f->sil[i] >= 0 && name == f->sil[i]) gSilObj[i] = o;
    if (!is_a_cls(cls, gClsMover)) return 0;
    tag = (int)rd32(o + offTag);
    for (i = 0; i < NDOORS && i < MAXGATES; i++) if (f->names[i] >= 0 && tag == f->names[i] && f->nfound[i] < GATEMOVERS) f->found[i][f->nfound[i]++] = o;
    return 0;
}
/* First-visit vs replay cutscenes. Story maps pair a first-visit CutSceneTrigger (bPersistent, fires once, event E)
   with a replay one on the same Tag (event E_02..., ExcludeFromGameStates GState000). Vanilla plays the map first in
   GState000, where only the first one runs, and on later visits the first one has already fired (it saves
   bInitiallyActive = false as <LevelID><Name>_bTOO). Open Castle enters in GState090, so on a first visit BOTH run
   and every Trigger in them happens twice: a toggle mover opens and shuts again (Ch1: the imp portrait springs back,
   the trapdoor over the Spongify pad closes, softlock).
   On each new map, for every pair of CutSceneTriggers with the same Tag where the replay's Event is the first one's
   plus "_02", or the pair is in CS_PAIRS below: while the first-visit one is still active (hasn't fired in this
   save), the REPLAY one loses its Event, so the first visit plays as vanilla's first visit did (dialogue and its
   rewards: 6035Ch1HoleQC also fires SpawnPastiesImp, the pasties the replay version leaves out; poc8e had it the other
   way round and lost them). Once the first-visit one has fired, nothing is changed and the replay plays, as in vanilla.
   Retail maps: HP3_Ch1CarpeRetractum 6033Ch1GnomeGetsUp, 6035Ch1HoleQC, 6040Ch1PixiesQC, 6045Ch1BeatPixiesQC (each
   with _02); HP3_Ch3Glacius 7550SP3GlaciusDirec / GlaciusDirecV2 (both trigger the tutorial spike gates
   Glacius_Tut_SpkGate_01/02, movers Glacius_Tut_Mvr_01/02 and two lights, so together they lowered the gates and raised
   them again).
   poc8o tried a general rule instead (same Tag, one bPersistent and one not). It also matched triggers that belong to
   other game states: those aren't removed at load, they stay in the level, so on InsideHub it switched off five
   unrelated ones (moving stairs 03To05 among them). poc8q is back to names: the "_02" rule plus this list. */
static const wchar *const CS_PAIRS[][2] = {
    { L"7550SP3GlaciusDirec", L"GlaciusDirecV2" },   /* HP3_Ch3Glacius, Tag 7550SP3GlaciusDirecCS */
};
#define NCS_PAIRS (int)(sizeof CS_PAIRS / sizeof CS_PAIRS[0])
#define CS_MAX 256
struct csfind { u32 level, cls; int n; u32 obj[CS_MAX]; int tag[CS_MAX], ev[CS_MAX]; };
static int v_cs(u32 o, int name, u32 cls, void *c) {
    struct csfind *f = c; (void)name;
    if (!cls || f->n >= CS_MAX || rd32(o + offLevel) != f->level || !is_a_cls(cls, f->cls)) return 0;
    f->obj[f->n] = o; f->tag[f->n] = (int)rd32(o + offTag); f->ev[f->n] = (int)rd32(o + offEvent); f->n++;
    return 0;
}
static int cs_is_replay(const wchar *first, const wchar *e) {   /* e = first + "_02...", or a CS_PAIRS entry */
    int k;
    for (k = 0; k < NCS_PAIRS; k++) if (wieq(first, CS_PAIRS[k][0]) && wieq(e, CS_PAIRS[k][1])) return 1;
    for (k = 0; first[k] && wlow(first[k]) == wlow(e[k]); k++) ;
    return !first[k] && e[k] == '_' && e[k + 1] == '0' && e[k + 2] == '2';   /* not "_010", a state variant */
}
static void dedupe_cutscenes(u32 level) {
    static struct csfind f; static u32 clsCS, maskActive, maskInState; static int offActive = -1, offInState = -1; static wchar b[220]; int i, j, n = 0;
    if (!clsCS) clsCS = find_class(L"CutSceneTrigger");
    if (offEvent < 0) offEvent = prop_offset(L"Actor", L"Event");
    if (offActive <= 0) {   /* Trigger.bInitiallyActive; UBoolProperty.BitMask at +0x70 */
        u32 bp = prop_object(L"Trigger", L"bInitiallyActive"), m = bp ? rd32(bp + 0x70) : 0;
        if (bp && m && !(m & (m - 1))) { offActive = (int)rd32(bp + 0x4C); maskActive = m; } else offActive = 0;
    }
    if (offInState <= 0) {   /* Actor.bInCurrentGameState (Actor.FilterForCurrentGameState clears it for actors of other states) */
        u32 bp = prop_object(L"Actor", L"bInCurrentGameState"), m = bp ? rd32(bp + 0x70) : 0;
        if (bp && m && !(m & (m - 1)) && rd32(bp + 0x4C) >= 0x28) { offInState = (int)rd32(bp + 0x4C); maskInState = m; } else offInState = 0;
    }
    if (!clsCS || offEvent <= 0 || offTag <= 0 || offActive <= 0 || offInState <= 0) { log_line(L"cutscenes: lookups failed; pairs left alone"); return; }
    f.level = level; f.cls = clsCS; f.n = 0;
    for_each_object(v_cs, &f);
    for (i = 0; i < f.n; i++) {
        wchar a[96]; int first_pending;
        if (!f.ev[i] || !name_of(f.ev[i], a, 96)) continue;
        for (j = 0; j < f.n; j++) {
            wchar e[96];
            if (j == i || f.tag[j] != f.tag[i] || !f.ev[j] || !name_of(f.ev[j], e, 96) || !cs_is_replay(a, e)) continue;
            /* the first-visit trigger must belong to this game state: one grouped to another state (Ch1's
               6045Ch1BeatPixiesQC is in group GState000) is hidden and never fires, so its replay is the only one
               (poc8s and earlier switched that replay off: softlock after the Carpe doxy fight) */
            if (!(rd32(f.obj[i] + offInState) & maskInState)) {
                wsprintfW(b, L"cutscenes: %s isn't in this game state; %s plays", a, e); log_line(b); break;
            }
            first_pending = (rd32(f.obj[i] + offActive) & maskActive) != 0;
            if (first_pending && rd32(f.obj[j] + offEvent) == (u32)f.ev[j]) {
                *(volatile int *)(f.obj[j] + offEvent) = 0;
                wsprintfW(b, L"cutscenes: first visit, %s plays; replay %s off", a, e); log_line(b); n++;
            } else if (!first_pending) { wsprintfW(b, L"cutscenes: %s already played in this save; %s plays", a, e); log_line(b); }
            break;
        }
    }
    if (n) { wsprintfW(b, L"cutscenes: %d replay trigger(s) switched off on this map (first visit)", n); log_line(b); }
}

static void chamber_poll(u32 level, const wchar *map);   /* chamber.h */
/* every 2 s: read which doors are unlocked; raise/lower each gate on this map to match */
static void gates_poll(u32 level, const wchar *map) {
    static wchar b[200]; int i, changed = 0;
    for (i = 0; i < NDOORS; i++) {
        int on;
        gIniDoor[i] = 0;   /* no ini start state any more (poc8c): locked until found */
        on = gIniDoor[i] || gSaveDoor[i];
        if (on != gDoorOn[i]) { changed = 1; gDoorOn[i] = on; }
    }
    if (changed) {
        wcpy(b, L"doors unlocked:", 200);
        for (i = 0; i < NDOORS; i++) if (gDoorOn[i]) wsprintfW(b + wlen(b), L" %s", DOORS[i].key);
        log_line(b);
    }
    if (!gClsMover || offTag < 0 || offKeyNum < 0) return;
    if (level != gGateLevel) dedupe_cutscenes(level);
    if (level != gGateLevel) {   /* new map: find its gate movers */
        struct findgate f; int n = 0;
        f.level = level;
        for (i = 0; i < MAXGATES; i++) { int k; for (k = 0; k < GATEMOVERS; k++) f.found[i][k] = 0; f.nfound[i] = 0; f.barfound[i] = 0; f.names[i] = f.bars[i] = -1; gGateLast[i] = 0;
            f.trigs[i][0] = f.trigs[i][1] = -1; f.trigfound[i][0] = f.trigfound[i][1] = 0; gTrigApplied[i] = -1; }
        for (i = 0; i < NDOORS && i < MAXGATES; i++) if (weq(DOORS[i].map, map)) {
            if (DOORS[i].gate) { f.names[i] = name_index(DOORS[i].gate); n++; }
            if (DOORS[i].bar)  { f.bars[i]  = name_index(DOORS[i].bar);  n++; }
            if (DOORS[i].trig[0]) { f.trigs[i][0] = name_index(DOORS[i].trig[0]); n++; }
            if (DOORS[i].trig[1]) { f.trigs[i][1] = name_index(DOORS[i].trig[1]); n++; }
        }
        for (i = 0; i < NSUPPRESS; i++) { gSuppObj[i] = 0; f.supp[i] = weq(SUPPRESS[i].map, map) ? name_index(SUPPRESS[i].actor) : -1; if (f.supp[i] >= 0) n++; }
        for (i = 0; i < NSILENCE; i++) { gSilObj[i] = 0; f.sil[i] = weq(SILENCE[i].map, map) ? name_index(SILENCE[i].actor) : -1; if (f.sil[i] >= 0) n++; }
        if (n) for_each_object(v_findgate, &f);
        if (offEvent < 0) offEvent = prop_offset(L"Actor", L"Event");
        for (i = 0; i < NSUPPRESS; i++) {
            int ev, want;
            gSuppEvent[i] = 0;
            if (!gSuppObj[i] || offEvent <= 0 || offTag <= 0) continue;
            ev = (int)rd32(gSuppObj[i] + (SUPPRESS[i].tag ? offTag : offEvent)); want = name_index(SUPPRESS[i].event);
            if (want >= 0 && (ev == want || ev == 0)) { gSuppEvent[i] = want; wsprintfW(b, L"trigger %s found: %s", SUPPRESS[i].actor, SUPPRESS[i].why); }
            else { gSuppObj[i] = 0; wsprintfW(b, L"trigger %s has an unexpected Event; left alone", SUPPRESS[i].actor); }
            log_line(b);
        }
        for (i = 0; i < MAXGATES; i++) { int k; for (k = 0; k < GATEMOVERS; k++) gGateObj[i][k] = f.found[i][k]; gBarObj[i] = f.barfound[i];
            gTrigObj[i][0] = f.trigfound[i][0]; gTrigObj[i][1] = f.trigfound[i][1]; }
        gGateLevel = level;
        for (i = 0; i < NSILENCE; i++) if (gSilObj[i]) { wsprintfW(b, L"voice line off: %s %s", SILENCE[i].actor, SILENCE[i].why); log_line(b); gSilFire = 1; }
        for (i = 0; i < NDOORS && i < MAXGATES; i++) {
            if (f.names[i] >= 0) { wsprintfW(b, L"gate %s: %s, %d mover(s)", DOORS[i].key, DOORS[i].gate, f.nfound[i]); log_line(b); }
            if (f.bars[i] >= 0)  { wsprintfW(b, L"bar %s: %s %s", DOORS[i].key, DOORS[i].bar, f.barfound[i] ? L"found" : L"NOT FOUND"); log_line(b); }
            if (f.trigs[i][0] >= 0) { wsprintfW(b, L"loading trigger %s: %s %s%s%s", DOORS[i].key, DOORS[i].trig[0], f.trigfound[i][0] ? L"found" : L"NOT FOUND",
                                                DOORS[i].trig[1] ? L", second " : L"", DOORS[i].trig[1] ? (f.trigfound[i][1] ? L"found" : L"NOT FOUND") : L""); log_line(b); }
        }
    }
    for (i = 0; i < NDOORS && i < MAXGATES; i++) {   /* lock bars: hidden while the door is unlocked */
        u8 want = gDoorOn[i] ? 0 : 8, cur;
        if (!gBarObj[i] || offDrawType <= 0) continue;
        cur = *(volatile u8 *)(gBarObj[i] + offDrawType);
        if (cur == want || (cur != 0 && cur != 8)) continue;   /* only ever toggle between the two we know */
        *(volatile u8 *)(gBarObj[i] + offDrawType) = want;
        wsprintfW(b, L"bar %s: %s", DOORS[i].key, want ? L"shown (locked)" : L"hidden (unlocked)"); log_line(b);
    }
    for (i = 0; i < NDOORS && i < MAXGATES; i++) {   /* the door's own loading triggers: collision on only while unlocked */
        if (!gTrigObj[i][0] && !gTrigObj[i][1]) continue;
        if (gTrigApplied[i] == gDoorOn[i]) continue;
        gTrigWant[i] = gDoorOn[i]; gTrigApplied[i] = gDoorOn[i]; gTrigFire |= 1u << i;
        wsprintfW(b, L"loading trigger %s: %s", DOORS[i].key, gDoorOn[i] ? L"on" : L"off"); log_line(b);
    }
    for (i = 0; i < NSUPPRESS; i++) {   /* event gates: the trigger's Event follows its key (always off without one) */
        int on = 0, k, cur, want;
        if (!gSuppObj[i] || !gSuppEvent[i]) continue;
        if (SUPPRESS[i].key) for (k = 0; k < NDOORS; k++) if (weq(DOORS[k].key, SUPPRESS[i].key)) { on = gDoorOn[k]; break; }
        want = on ? gSuppEvent[i] : 0;
        cur = (int)rd32(gSuppObj[i] + (SUPPRESS[i].tag ? offTag : offEvent));
        if (cur == want) continue;
        *(volatile int *)(gSuppObj[i] + (SUPPRESS[i].tag ? offTag : offEvent)) = want;
        wsprintfW(b, L"trigger %s (%s): %s", SUPPRESS[i].actor, SUPPRESS[i].why, want ? L"on" : L"off"); log_line(b);
    }
    for (i = 0; i < NDOORS && i < MAXGATES; i++) {
        u32 m = gGateObj[i][0]; int open;
        if (!m) continue;
        open = (rd32(m + offKeyNum) & 0xFF) != 0;   /* InterpolateTo sets KeyNum to the target key at once */
        if (DOORS[i].inv) open = !open;
        if (open == gDoorOn[i] || (gGateLast[i] && GetTickCount() - gGateLast[i] < 3000)) continue;
        gGateLast[i] = GetTickCount(); gGateFire |= 1u << i;
        wsprintfW(b, L"gate %s: %s", DOORS[i].key, gDoorOn[i] ? L"raising" : L"lowering"); log_line(b);
    }
    chamber_poll(level, map);   /* the B maps' challenge-chamber copies */
}

#include "shop.h"
#include "unlocks.h"
#include "aptest.h"
#include "comcfix.h"
#include "blocker.h"
#include "panel.h"
#include "noclip.h"
#include "chamber.h"
#include "net.h"
#include "ap.h"
#include "aplook.h"
#include "apshop.h"

static u32 WINAPI worker(void *arg) {
    HANDLE core; static wchar buf[400], id[32], lastMap[64]; static struct players pl;
    int i, cur = 0, armed = 0, settle = 0, shortcutsDone = 1; u32 clsPlayer, pc, pawn, level; float loc[3];
    (void)arg;
    gProc = GetCurrentProcess();
    {   wchar docs[520];
        if (SHGetFolderPathW(0, 5 /*CSIDL_PERSONAL*/, 0, 0, docs) == 0)
            wsprintfW(gLogPath, L"%s\\Harry Potter and the Prisoner of Azkaban\\HP3_OpenCastle.log", docs);
        else wcpy(gLogPath, L"HP3_OpenCastle.log", 600);
        if (SHGetFolderPathW(0, 5, 0, 0, docs) == 0)
            wsprintfW(gIniPath, L"%s\\Harry Potter and the Prisoner of Azkaban\\HP3_OpenCastle.ini", docs);
        else wcpy(gIniPath, L".\\HP3_OpenCastle.ini", 600);
        if (SHGetFolderPathW(0, 5, 0, 0, docs) == 0)
            wsprintfW(gMapLogPath, L"%s\\Harry Potter and the Prisoner of Azkaban\\HP3_MapLog.txt", docs);
        for (i = 0; i < 16; i++) gDoorOn[i] = 0;   /* locked until the save says otherwise */
    }
    wsprintfW(buf, L"---- HP3 Open Castle " OC_VERSION L" (" OC_BUILD L") loaded: travel hook %s, ProcessEvent hook %s ----",
              gHookFurl ? L"ok" : L"NOT INSTALLED", gHookPE ? L"ok" : L"NOT INSTALLED");
    log_line(buf);
    while (!(core = GetModuleHandleW(L"Core.dll"))) Sleep(500);
    gObjs  = (u32 *)GetProcAddress(core, "?GObjObjects@UObject@@0V?$TArray@PAVUObject@@@@A");
    gNames = (u32 *)GetProcAddress(core, "?Names@FName@@0V?$TArray@PAUFNameEntry@@@@A");
    if (!gObjs || !gNames) { log_line(L"Core.dll exports missing; doors disabled"); return 1; }
    for (;;) {   /* wait for the engine's names */
        Sleep(1000);
        if (rd32((u32)gObjs + 4) < 1000) continue;
        if ((N_Class = name_index(L"Class")) >= 0) break;
    }
    for (;;) {
        Sleep(250);
        {   /* names: some (HGame's) only exist once their package has loaded, so retry the missing ones every 2 s */
            static int tick; static int logged;
            if (tick++ % 8 == 0) {
                int miss = 0;
#define NAME(v, s) if (v < 0) { v = name_index(s); if (v < 0) miss++; }
                NAME(N_ServerTravel, L"ServerTravel") NAME(N_RelocatePawn, L"RelocatePawn")
                NAME(N_EnableShortcuts, L"EnablePortraitShortcuts") NAME(N_SaveValue, L"SaveValue")
                NAME(N_RestoreValue, L"RestoreValue") NAME(N_FancySpawn, L"FancySpawn") NAME(N_Destroyed, L"Destroyed")
                NAME(N_PostPDR, L"PostPersistentDataRestored") NAME(N_TravelExporting, L"TravelExporting")
                NAME(N_Trigger, L"Trigger") NAME(N_PlayerTick, L"PlayerTick") NAME(N_Tick, L"Tick")
#undef NAME
                if (!miss && !logged) {
                    logged = 1;
                    wsprintfW(buf, L"names: ServerTravel=%d RelocatePawn=%d EnablePortraitShortcuts=%d SaveValue=%d RestoreValue=%d "
                              L"FancySpawn=%d Destroyed=%d PostPersistentDataRestored=%d TravelExporting=%d", N_ServerTravel,
                              N_RelocatePawn, N_EnableShortcuts, N_SaveValue, N_RestoreValue, N_FancySpawn, N_Destroyed, N_PostPDR,
                              N_TravelExporting);
                    log_line(buf);
                }
            }
        }
        if (offPlayerActor < 0) offPlayerActor = prop_offset(L"Player", L"Actor");
        if (offPawn < 0)  offPawn  = prop_offset(L"Controller", L"Pawn");
        if (offLoc < 0)   offLoc   = prop_offset(L"Actor", L"Location");
        if (offLevel < 0) offLevel = prop_offset(L"Actor", L"Level");
        if (offID < 0)    offID    = prop_offset(L"LevelInfo", L"ID");
        if (offPrevLevel < 0) offPrevLevel = prop_offset(L"KWPawn", L"PreviousLevelName");
        if (offGame < 0)  offGame  = prop_offset(L"LevelInfo", L"Game");
        if (offPPM < 0)   offPPM   = prop_offset(L"HGame", L"PortraitPasswordManager");
        if (!gClsHero && (gClsHero = find_class(L"KWHeroController")) != 0) N_TravelPostAccept = name_index(L"TravelPostAccept");
        if (offLives < 0)   offLives   = prop_offset(L"KWGenericSpawner", L"Lives");
        if (offGoodies < 0) offGoodies = prop_offset(L"KWGenericSpawner", L"GoodieToSpawn");
        if (offPicked < 0)  offPicked  = prop_offset(L"KWPickup", L"HasBeenPickedUp");
        if (offStore < 0)   offStore   = prop_offset(L"GameInfo", L"PersistentStore");
        if (!gClsPickup) gClsPickup = find_class(L"KWPickup");
        if (offTag < 0)    offTag    = prop_offset(L"Actor", L"Tag");
        if (offKeyNum < 0) offKeyNum = prop_offset(L"Mover", L"KeyNum");
        if (offDrawType < 0) offDrawType = prop_offset(L"Actor", L"DrawType");
        if (!gClsMover) gClsMover = find_class(L"Mover");
        /* PostBeginPlay is armed last: the leftover tracking only starts once everything it needs is known */
        if (!gClsSpawner && gClsPickup && offLives > 0 && offGoodies > 0 && offPicked > 0 && offStore > 0 && offLoc > 0
            && offLevel >= 0 && (gClsSpawner = find_class(L"KWGenericSpawner")) != 0) N_PostBeginPlay = name_index(L"PostBeginPlay");
        if (!gPlayer) {
            clsPlayer = find_class(L"Player");
            if (clsPlayer) {
                pl.clsPlayer = clsPlayer; pl.clsClass = rd32(clsPlayer + 0x24); pl.n = 0; pl.nknown = 0;
                for_each_object(v_players, &pl);
                for (i = 0; i < pl.n; i++) if (offPlayerActor >= 0 && rd32(pl.found[i] + offPlayerActor)) { gPlayer = pl.found[i]; break; }
                if (gPlayer) {
                    wsprintfW(buf, L"ready: offsets player.actor=%d pawn=%d loc=%d level=%d id=%d prevlevel=%d game=%d ppm=%d; "
                              L"names ServerTravel=%d TravelPostAccept=%d RelocatePawn=%d EnablePortraitShortcuts=%d; hero class %s; "
                              L"leftovers: lives=%d goodies=%d picked=%d store=%d spawner %s pickup %s",
                              offPlayerActor, offPawn, offLoc, offLevel, offID, offPrevLevel, offGame, offPPM,
                              N_ServerTravel, N_TravelPostAccept, N_RelocatePawn, N_EnableShortcuts, gClsHero ? L"ok" : L"MISSING",
                              offLives, offGoodies, offPicked, offStore, gClsSpawner ? L"ok" : L"MISSING", gClsPickup ? L"ok" : L"MISSING");
                    log_line(buf);
                }
            }
            continue;
        }
        {   static int hubState;   /* 0 not tried, 1 ready, -1 disabled */
            if (!hubState) hubState = hub_init() ? 1 : -1;
            if (hubState > 0) hub_poll();
        }
        pc = rd32(gPlayer + offPlayerActor);
        level = pc ? rd32(pc + offLevel) : 0;
        if (!level || !read_fstring(level + offID, id, 32) || !map_for_id(id)) continue;
        if (!weq(map_for_id(id), lastMap)) {
            wcpy(lastMap, map_for_id(id), 64);
            cur ^= 1; wcpy(gMapBuf[cur], lastMap, 64); gCurMap = gMapBuf[cur];
            wsprintfW(buf, L"MAP %s", lastMap); log_line(buf);
            armed = 0; settle = 0; shortcutsDone = 0;
            if (gNewGame) { gNewGame = 0; unlocks_reset(); }
        }
        settle++;
        ap_worker(level, lastMap, settle);
        if (settle >= 4) comc_poll(level, lastMap);
        if (settle >= 12 && (settle - 12) % 8 == 0) { gates_poll(level, lastMap); blockers_poll(level, lastMap); shop_poll(level, offGame > 0 ? rd32(level + offGame) : 0); spells_init(); unlocks_poll(); if (OC_DEBUG) { panel_lookups(); noclip_lookups(); } }   /* once settled, every 2 s */
        if (!shortcutsDone && settle >= 12) {
            shortcutsDone = 1;
            if (!weq(lastMap, L"Entry") && !weq(lastMap, L"HP_Preamble")) { gPendingShortcuts = 1; gPendingRespawn = 1; }
        }
        pawn = rd32(pc + offPawn);
        if (settle >= 12) grid_poll(pawn, level);
        if (!pawn || !rd(pawn + offLoc, loc, 12)) { gAtTransfigDoor = 0; continue; }
        {   float dx = loc[0] + 3171.0f, dy = loc[1] - 2204.0f;   /* Transfiguration exam door, ChangeLevelTrigger7 */
            gAtTransfigDoor = weq(lastMap, L"HP3_InsideHub") && dx * dx + dy * dy < 250.0f * 250.0f;
        }
        if (gPending) continue;
        {   int near = -1, far_from_all = 1;
            for (i = 0; i < NDOORS; i++) {
                float dx, dy, dz, h2;
                if (!DOORS[i].dest || !weq(DOORS[i].map, lastMap)) continue;
                dx = loc[0] - DOORS[i].x; dy = loc[1] - DOORS[i].y; dz = loc[2] - DOORS[i].z; h2 = dx * dx + dy * dy;
                if (h2 < ARM_R * ARM_R) far_from_all = 0;
                if (h2 < DOOR_R * DOOR_R && dz < DOOR_DZ && dz > -DOOR_DZ && gDoorOn[i]) near = i;
            }
            if (far_from_all && !armed) armed = 1;
            if (near >= 0 && armed) {
                wcpy(gPendingURL, DOORS[near].dest, 128);
                wsprintfW(buf, L"at %s -> %s", DOORS[near].name, DOORS[near].dest); log_line(buf);
                armed = 0; gPending = 1;
            }
        }
    }
}

BOOL WINAPI DllMain(HANDLE inst, u32 reason, void *res) {
    (void)res;
    if (reason == 1) {
        DisableThreadLibraryCalls(inst);
        install_hooks();   /* on the loading thread, before the worker starts */
        CreateThread(0, 0, worker, 0, 0, 0);
        CreateThread(0, 0, net_thread, 0, 0, 0);   /* the Archipelago client connection (net.h) */
        if (OC_DEBUG) CreateThread(0, 0, key_thread, 0, 0, 0);   /* debug panel, test grid, noclip */
    }
    return 1;
}
