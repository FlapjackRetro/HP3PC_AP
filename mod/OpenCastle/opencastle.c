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
 * Not in this build: item gating (every door is open), return points, spell hooks, the Marauder's Map pickup.
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

#define OC_VERSION L"poc1"

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
static int N_ServerTravel = -1;
static volatile int gPending; static wchar gPendingURL[128];

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
static void __thiscall pe_hook(void *self, void *func, void *parms, void *result) {
    if (gPending && !gInTravel) { gInTravel = 1; gPending = 0; do_travel(); gInTravel = 0; }
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
struct door { const wchar *map; float x, y, z; const wchar *dest; const wchar *name; };
static const struct door DOORS[] = {
    { L"HP3_InsideHub",  731, 1615,   74, L"HP3_Adv6Paddock",     L"entrance hall north door" },
    { L"HP3_InsideHub", 1427, -960, 2154, L"HP3_Adv1Express",     L"7th floor east door" },
    { L"HP3_InsideHub",  838, -1844, 1194, L"HP3_Adv2ExpPatronum", L"DADA portcullis" },
    { L"HP3_InsideHub", -710, -1349,  778, L"HP3_Adv3Library",     L"Library portcullis" },
    { L"HP3_DungeonHUB", -481, 2805, -341, L"HP3_Adv5Shack",       L"dungeon door 1" },
};
#define NDOORS (int)(sizeof DOORS / sizeof DOORS[0])
#define DOOR_R   90.0f    /* horizontal reach */
#define DOOR_DZ 160.0f
#define ARM_R   300.0f    /* must have been this far from every door on the map before a door works */

static u32 WINAPI worker(void *arg) {
    HANDLE core; static wchar buf[400], id[32], lastMap[64]; static struct players pl;
    int i, cur = 0, armed = 0; u32 clsPlayer, pc, pawn, level; float loc[3];
    (void)arg;
    gProc = GetCurrentProcess();
    {   wchar docs[520];
        if (SHGetFolderPathW(0, 5 /*CSIDL_PERSONAL*/, 0, 0, docs) == 0)
            wsprintfW(gLogPath, L"%s\\Harry Potter and the Prisoner of Azkaban\\HP3_OpenCastle.log", docs);
        else wcpy(gLogPath, L"HP3_OpenCastle.log", 600);
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
    N_ServerTravel = name_index(L"ServerTravel");
    for (;;) {
        Sleep(250);
        if (offPlayerActor < 0) offPlayerActor = prop_offset(L"Player", L"Actor");
        if (offPawn < 0)  offPawn  = prop_offset(L"Controller", L"Pawn");
        if (offLoc < 0)   offLoc   = prop_offset(L"Actor", L"Location");
        if (offLevel < 0) offLevel = prop_offset(L"Actor", L"Level");
        if (offID < 0)    offID    = prop_offset(L"LevelInfo", L"ID");
        if (!gPlayer) {
            clsPlayer = find_class(L"Player");
            if (clsPlayer) {
                pl.clsPlayer = clsPlayer; pl.clsClass = rd32(clsPlayer + 0x24); pl.n = 0; pl.nknown = 0;
                for_each_object(v_players, &pl);
                for (i = 0; i < pl.n; i++) if (offPlayerActor >= 0 && rd32(pl.found[i] + offPlayerActor)) { gPlayer = pl.found[i]; break; }
                if (gPlayer) {
                    wsprintfW(buf, L"ready: offsets player.actor=%d pawn=%d loc=%d level=%d id=%d; ServerTravel name %d",
                              offPlayerActor, offPawn, offLoc, offLevel, offID, N_ServerTravel);
                    log_line(buf);
                }
            }
            continue;
        }
        pc = rd32(gPlayer + offPlayerActor);
        level = pc ? rd32(pc + offLevel) : 0;
        if (!level || !read_fstring(level + offID, id, 32) || !map_for_id(id)) continue;
        if (!weq(map_for_id(id), lastMap)) {
            wcpy(lastMap, map_for_id(id), 64);
            cur ^= 1; wcpy(gMapBuf[cur], lastMap, 64); gCurMap = gMapBuf[cur];
            wsprintfW(buf, L"MAP %s", lastMap); log_line(buf);
            armed = 0;
        }
        pawn = rd32(pc + offPawn);
        if (!pawn || !rd(pawn + offLoc, loc, 12) || gPending) continue;
        {   int near = -1, far_from_all = 1;
            for (i = 0; i < NDOORS; i++) {
                float dx, dy, dz, h2;
                if (!weq(DOORS[i].map, lastMap)) continue;
                dx = loc[0] - DOORS[i].x; dy = loc[1] - DOORS[i].y; dz = loc[2] - DOORS[i].z; h2 = dx * dx + dy * dy;
                if (h2 < ARM_R * ARM_R) far_from_all = 0;
                if (h2 < DOOR_R * DOOR_R && dz < DOOR_DZ && dz > -DOOR_DZ) near = i;
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
    }
    return 1;
}
