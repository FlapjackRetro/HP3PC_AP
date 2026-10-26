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
 *  5. Shortcut passwords start unlocked (PortraitPasswordManager.EnablePortraitShortcuts, once per map load).
 *  6. Leftover goodies (leftovers.h): items a chest threw out but the player didn't pick up come back on the next
 *     visit.
 *  7. Softlock escape (hubmenu.h): RETURN TO HUB in the pause menu's exit dialog.
 *  8. Door gates: a door's loading trigger only works while the door is unlocked, and its portcullis (a TriggerToggle
 *     KWMover, the one the vanilla cutscenes trigger) is raised or lowered to match. Which doors are unlocked comes
 *     from Documents\Harry Potter and the Prisoner of Azkaban\HP3_OpenCastle.ini, [Doors] <key>=1/0 (default 1), read
 *     every 2 s, so it can be changed while the game runs. The randomizer will drive the same switch.
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

#define OC_VERSION L"poc6n"

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

/* ---- hook 1: FURL::FURL(FURL* Base, const TCHAR* TextURL, ETravelType) ---- */
typedef void *(__thiscall *furl_fn)(void *self, void *base, const wchar *text, int type);
static furl_fn gFurlOrig;
static wchar gUrlOut[600];
static void *__thiscall furl_hook(void *self, void *base, const wchar *text, int type) {
    const wchar *use = text; wchar why[128]; static wchar msg[900];
    if (text && oc_rewrite((const wchar *)gCurMap, text, gUrlOut, 600, why, 128)) {
        use = gUrlOut;
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
    { L"HP3_InsideHub",  L"HP3_FA1Ron",          L"HP3_Ch1CarpeRetractumB",   0,    0,     0,    0 },   /* exam side room */
    { L"HP3_InsideHub",  L"HP3_FA2Hermione",     L"HP3_Ch2DraconiforsB",      0,    0,     0,    0 },
    { L"HP3_InsideHub",  L"HP3_FA3Harry",        L"HP3_Ch3GlaciusB",          0,    0,     0,    0 },
    { L"HP3_DungeonHUB", L"HP3_Adv5Shack",       0,                           1, -481,  2805, -341 },   /* no SmartStarts */
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
    if (!pawn || !hub || offPrevLevel < 0 || !rd(pawn + offPrevLevel, sv, 12)) { gPEOrig(self, func, parms, result); return; }
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
#define MAXGATES 8
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
static volatile int gGridPending; static void grid_spawn(void);
static volatile u32 gBlockSpawn, gBlockKill; static void blockers_apply(void);
static void __thiscall pe_hook(void *self, void *func, void *parms, void *result) {
    int fname;
    if (gPending && !gInTravel) { gInTravel = 1; gPending = 0; do_travel(); gInTravel = 0; }
    if (gPendingShortcuts && !gInTravel) { gInTravel = 1; gPendingShortcuts = 0; enable_shortcuts(); gInTravel = 0; }
    if (gPendingRespawn && !gInTravel) { gInTravel = 1; gPendingRespawn = 0; lo_respawn(); gInTravel = 0; }
    if (gMenuPatch && !gInTravel) { gInTravel = 1; gMenuPatch = 0; hub_patch(); gInTravel = 0; }
    if (gGateFire && !gInTravel) { gInTravel = 1; fire_gates(); gInTravel = 0; }
    if (gGridPending && !gInTravel) { gInTravel = 1; grid_spawn(); gGridPending = 0; gInTravel = 0; }
    if ((gBlockSpawn || gBlockKill) && !gInTravel) { gInTravel = 1; blockers_apply(); gInTravel = 0; }
    if (!func || !self) { gPEOrig(self, func, parms, result); return; }
    fname = *(int *)((u8 *)func + 0x20);
    if (fname == N_MyOnClick && N_MyOnClick >= 0 && !gInTravel) {
        int done; gInTravel = 1; done = hub_click((u32)self, (u32 *)parms); gInTravel = 0;
        if (done) return;
    }
    if (fname == N_PostBeginPlay || fname == N_Destroyed || fname == N_PostPDR || fname == N_TravelExporting)
        lo_event((u32)self, fname);
    if (fname == N_TravelPostAccept && N_TravelPostAccept >= 0 && gClsHero
        && is_a_cls(*(u32 *)((u8 *)self + 0x24), gClsHero)) { travel_post_accept(self, func, parms, result); return; }
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
struct door { const wchar *map; float x, y, z; const wchar *dest; const wchar *name; const wchar *key; const wchar *gate; const wchar *bar; };
static const struct door DOORS[] = {
    /* map               stand here            goes to                 name                        ini key    portcullis mover tag      lock bar (StaticMeshActor, DoorLockBar_SM) */
    { L"HP3_InsideHub",  731, 1615,   74, L"HP3_Adv6Paddock",     L"entrance hall north door", L"Paddock", 0,                        L"StaticMeshActor42" },
    { L"HP3_InsideHub", 1427, -960, 2154, L"HP3_Adv1Express",     L"7th floor east door",      L"Express", 0,                        L"StaticMeshActor228" },
    { L"HP3_InsideHub",  838, -1844, 1194, L"HP3_Adv2ExpPatronum", L"DADA portcullis",         L"DADA",    L"DADA_PortToExpecto_Mvr", 0 },
    { L"HP3_InsideHub", -710, -1349,  778, L"HP3_Adv3Library",     L"Library portcullis",      L"Library", L"Library_Port_Mvr",       0 },
    { L"HP3_DungeonHUB", -481, 2805, -341, L"HP3_Adv5Shack",       L"dungeon door 1",          L"Shack",   0,                        L"StaticMeshActor315" },
    /* challenges: no loading trigger of ours (dest 0). The Monster Book room has a door (portcullis + two doors, all
       tag MBOMDoor, movable: 7200TaskedCharms opens them); the hippogriff and doxy challenges have no door, so their
       start triggers are switched off while locked (EVENTGATES). */
    { L"HP3_InsideHub",     0,     0,    0, 0,                     L"Monster Book challenge",  L"MonsterBook", L"MBOMDoor",          0 },
    { L"HP3_GroundsHub",    0,     0,    0, 0,                     L"hippogriff challenge",    L"Hippogriff",  0,                    0 },
    { L"HP3_GroundsHub",    0,     0,    0, 0,                     L"doxy challenge",          L"Doxy",        0,                    0 },
};
#define NDOORS (int)(sizeof DOORS / sizeof DOORS[0])
#define DOOR_R   90.0f    /* horizontal reach */
#define DOOR_DZ 160.0f
#define ARM_R   300.0f    /* must have been this far from every door on the map before a door works */

/* ---- door gates (worker part) ---- */
static wchar gIniPath[600];
static int gDoorOn[16];
static int offTag = -1, offKeyNum = -1;
static u32 gClsMover;
static u32 gGateLevel; static u32 gGateLast[MAXGATES];   /* GetTickCount of our last Trigger, per gate */
/* cutscene triggers Open Castle switches off (their Event is set to None once the map loads; the trigger stays but
   fires nothing). Only if the Event is the expected one, so a different map version is left alone. */
/* key 0: always off. key set: the trigger works only while that door / challenge is unlocked. */
struct suppress { const wchar *map, *actor, *event, *why, *key; };
static const struct suppress SUPPRESS[] = {
    /* 3rd floor corridor in front of DADA, GState090 group: 9240RonExamIntroPt1 = ChangeLevel HP3_FA1Ron + GState100 */
    { L"HP3_InsideHub",  L"CutSceneTrigger13",      L"9240RonExamIntroPt1",       L"Ron's exam intro on the 3rd floor (I1)", 0 },
    /* GroundsHub walk-in to the COMC course selection (CutSceneFunctionality group, every state) */
    { L"HP3_GroundsHub", L"CutSceneTrigger15",      L"ChangeLevel_COMC_MINIGAME", L"hippogriff challenge entrance",          L"Hippogriff" },
    /* the well: a companion spell starts the next doxy round (RoundRobin PixieBattleRoundRobin) */
    { L"HP3_GroundsHub", L"CompanionSpellTrigger10", L"PixieBattleRoundRobin",    L"doxy challenge start",                   L"Doxy" },
    /* the Monster Book cauldron: a companion spell starts the next round (RoundRobin MBRobin); backs up the door */
    { L"HP3_InsideHub",  L"CompanionSpellTrigger3",  L"MBRobin",                  L"Monster Book challenge start",           L"MonsterBook" },
};
#define NSUPPRESS (int)(sizeof SUPPRESS / sizeof SUPPRESS[0])
static int offEvent = -1;
static u32 gSuppObj[NSUPPRESS]; static int gSuppEvent[NSUPPRESS];   /* the trigger and its original Event name */

/* lock bars: separate StaticMeshActors (DoorLockBar_SM) in front of the doors without a portcullis. Static actors
   can't be moved, so an unlocked door's bar is hidden: DrawType DT_StaticMesh (8) -> DT_None (0). */
static int offDrawType = -1;
static u32 gBarObj[MAXGATES];
struct findgate { u32 level; int names[MAXGATES]; u32 found[MAXGATES][GATEMOVERS]; int nfound[MAXGATES]; int bars[MAXGATES]; u32 barfound[MAXGATES]; int supp[NSUPPRESS]; };
static int v_findgate(u32 o, int name, u32 cls, void *c) {
    struct findgate *f = c; int i, tag;
    if (!cls || rd32(o + offLevel) != f->level) return 0;
    for (i = 0; i < NDOORS && i < MAXGATES; i++) if (f->bars[i] >= 0 && name == f->bars[i]) f->barfound[i] = o;
    for (i = 0; i < NSUPPRESS; i++) if (f->supp[i] >= 0 && name == f->supp[i]) gSuppObj[i] = o;
    if (!is_a_cls(cls, gClsMover)) return 0;
    tag = (int)rd32(o + offTag);
    for (i = 0; i < NDOORS && i < MAXGATES; i++) if (f->names[i] >= 0 && tag == f->names[i] && f->nfound[i] < GATEMOVERS) f->found[i][f->nfound[i]++] = o;
    return 0;
}
/* every 2 s: read which doors are unlocked; raise/lower each gate on this map to match */
static void gates_poll(u32 level, const wchar *map) {
    static wchar b[200]; int i, changed = 0;
    for (i = 0; i < NDOORS; i++) {
        int on = GetPrivateProfileIntW(L"Doors", DOORS[i].key, 1, gIniPath) != 0;
        if (on != gDoorOn[i]) { changed = 1; gDoorOn[i] = on; }
    }
    if (changed) {
        wcpy(b, L"doors unlocked:", 200);
        for (i = 0; i < NDOORS; i++) if (gDoorOn[i]) wsprintfW(b + wlen(b), L" %s", DOORS[i].key);
        log_line(b);
    }
    if (!gClsMover || offTag < 0 || offKeyNum < 0) return;
    if (level != gGateLevel) {   /* new map: find its gate movers */
        struct findgate f; int n = 0;
        f.level = level;
        for (i = 0; i < MAXGATES; i++) { int k; for (k = 0; k < GATEMOVERS; k++) f.found[i][k] = 0; f.nfound[i] = 0; f.barfound[i] = 0; f.names[i] = f.bars[i] = -1; gGateLast[i] = 0; }
        for (i = 0; i < NDOORS && i < MAXGATES; i++) if (weq(DOORS[i].map, map)) {
            if (DOORS[i].gate) { f.names[i] = name_index(DOORS[i].gate); n++; }
            if (DOORS[i].bar)  { f.bars[i]  = name_index(DOORS[i].bar);  n++; }
        }
        for (i = 0; i < NSUPPRESS; i++) { gSuppObj[i] = 0; f.supp[i] = weq(SUPPRESS[i].map, map) ? name_index(SUPPRESS[i].actor) : -1; if (f.supp[i] >= 0) n++; }
        if (n) for_each_object(v_findgate, &f);
        if (offEvent < 0) offEvent = prop_offset(L"Actor", L"Event");
        for (i = 0; i < NSUPPRESS; i++) {
            int ev, want;
            gSuppEvent[i] = 0;
            if (!gSuppObj[i] || offEvent <= 0) continue;
            ev = (int)rd32(gSuppObj[i] + offEvent); want = name_index(SUPPRESS[i].event);
            if (want >= 0 && (ev == want || ev == 0)) { gSuppEvent[i] = want; wsprintfW(b, L"trigger %s found: %s", SUPPRESS[i].actor, SUPPRESS[i].why); }
            else { gSuppObj[i] = 0; wsprintfW(b, L"trigger %s has an unexpected Event; left alone", SUPPRESS[i].actor); }
            log_line(b);
        }
        for (i = 0; i < MAXGATES; i++) { int k; for (k = 0; k < GATEMOVERS; k++) gGateObj[i][k] = f.found[i][k]; gBarObj[i] = f.barfound[i]; }
        gGateLevel = level;
        for (i = 0; i < NDOORS && i < MAXGATES; i++) {
            if (f.names[i] >= 0) { wsprintfW(b, L"gate %s: %s, %d mover(s)", DOORS[i].key, DOORS[i].gate, f.nfound[i]); log_line(b); }
            if (f.bars[i] >= 0)  { wsprintfW(b, L"bar %s: %s %s", DOORS[i].key, DOORS[i].bar, f.barfound[i] ? L"found" : L"NOT FOUND"); log_line(b); }
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
    for (i = 0; i < NSUPPRESS; i++) {   /* event gates: the trigger's Event follows its key (always off without one) */
        int on = 0, k, cur, want;
        if (!gSuppObj[i] || !gSuppEvent[i]) continue;
        if (SUPPRESS[i].key) for (k = 0; k < NDOORS; k++) if (weq(DOORS[k].key, SUPPRESS[i].key)) { on = gDoorOn[k]; break; }
        want = on ? gSuppEvent[i] : 0;
        cur = (int)rd32(gSuppObj[i] + offEvent);
        if (cur == want) continue;
        *(volatile int *)(gSuppObj[i] + offEvent) = want;
        wsprintfW(b, L"trigger %s (%s): %s", SUPPRESS[i].actor, SUPPRESS[i].why, want ? L"on" : L"off"); log_line(b);
    }
    for (i = 0; i < NDOORS && i < MAXGATES; i++) {
        u32 m = gGateObj[i][0]; int open;
        if (!m) continue;
        open = (rd32(m + offKeyNum) & 0xFF) != 0;   /* InterpolateTo sets KeyNum to the target key at once */
        if (open == gDoorOn[i] || (gGateLast[i] && GetTickCount() - gGateLast[i] < 3000)) continue;
        gGateLast[i] = GetTickCount(); gGateFire |= 1u << i;
        wsprintfW(b, L"gate %s: %s", DOORS[i].key, gDoorOn[i] ? L"raising" : L"lowering"); log_line(b);
    }
}

#include "aptest.h"
#include "comcfix.h"
#include "blocker.h"

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
        for (i = 0; i < 16; i++) gDoorOn[i] = 1;
    }
    wsprintfW(buf, L"---- HP3 Open Castle " OC_VERSION L" loaded: travel hook %s, ProcessEvent hook %s ----",
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
                NAME(N_Trigger, L"Trigger")
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
        }
        settle++;
        if (settle >= 4) comc_poll(level, lastMap);
        if (settle >= 12 && (settle - 12) % 8 == 0) { gates_poll(level, lastMap); blockers_poll(level, lastMap); }   /* once settled, every 2 s */
        if (!shortcutsDone && settle >= 12) {
            shortcutsDone = 1;
            if (!weq(lastMap, L"Entry") && !weq(lastMap, L"HP_Preamble")) { gPendingShortcuts = 1; gPendingRespawn = 1; }
        }
        pawn = rd32(pc + offPawn);
        if (settle >= 12) grid_poll(pawn);
        if (!pawn || !rd(pawn + offLoc, loc, 12) || gPending) continue;
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
        CreateThread(0, 0, key_thread, 0, 0, 0);
    }
    return 1;
}
