/*
 * HP3 MapTracker - Ultimate ASI Loader plugin (plugins\HP3MapTracker.asi)
 *
 * Shows the current map and story game state in the game window's title bar and
 * appends every change to Documents\Harry Potter and the Prisoner of Azkaban\HP3_MapLog.txt.
 * Press F11 (with the game focused) to write a MARK line with the player's position.
 *
 * Read-only: it never writes game memory. All reads go through ReadProcessMemory on our own
 * process, so a stale pointer during a level change just fails instead of crashing.
 *
 * Engine layout (verified from Core.dll exports of the retail HP3 build):
 *   UObject: +0x18 Outer, +0x20 Name (FName index), +0x24 Class
 *   UStruct: +0x2C SuperField          FNameEntry: +0x0C wide-char name
 *   UProperty: +0x4C Offset            FString: { wchar_t* Data; int Num; int Max; }
 * No CRT; built with clang + lld-link (see build.sh).
 */
typedef unsigned short wchar;
typedef unsigned int   u32;
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
IMP u32    WINAPI GetCurrentProcessId(void);
IMP BOOL   WINAPI DisableThreadLibraryCalls(HANDLE);
IMP HANDLE WINAPI CreateFileW(const wchar*, u32, u32, void*, u32, u32, HANDLE);
IMP BOOL   WINAPI WriteFile(HANDLE, const void*, u32, u32*, void*);
IMP BOOL   WINAPI CloseHandle(HANDLE);
IMP void   WINAPI GetLocalTime(SYSTEMTIME*);
IMP int    WINAPI WideCharToMultiByte(u32, u32, const wchar*, int, char*, int, const char*, BOOL*);
IMP BOOL   WINAPI EnumWindows(BOOL (WINAPI*)(HWND, long), long);
IMP u32    WINAPI GetWindowThreadProcessId(HWND, u32*);
IMP BOOL   WINAPI IsWindowVisible(HWND);
IMP int    WINAPI GetWindowTextW(HWND, wchar*, int);
IMP long   WINAPI SendMessageTimeoutW(HWND, u32, u32, long, u32, u32, u32*);
IMP HWND   WINAPI GetForegroundWindow(void);
IMP short  WINAPI GetAsyncKeyState(int);
IMP int    __cdecl wsprintfW(wchar*, const wchar*, ...);
IMP long   WINAPI SHGetFolderPathW(HWND, int, HANDLE, u32, wchar*);

#define WM_SETTEXT 0x000C
#define SMTO_ABORTIFHUNG 0x0002
#define VK_F11 0x7A
#define MT_VERSION L"v4"

/* minimal CRT shims (no C runtime is linked) */
int _fltused = 0;
void *memset(void *d, int c, unsigned n) { volatile unsigned char *p = (volatile unsigned char *)d; while (n--) *p++ = (unsigned char)c; return d; }

static HANDLE gProc;
static u32 *gObjs;     /* &GObjObjects : { UObject** Data; int Num; int Max } */
static u32 *gNames;    /* &FName::Names : { FNameEntry** Data; int Num; int Max } */

static int rd(u32 addr, void *out, u32 n) { u32 got = 0; return addr && ReadProcessMemory(gProc, (void*)addr, out, n, &got) && got == n; }
static u32 rd32(u32 addr) { u32 v = 0; rd(addr, &v, 4); return v; }

static int weq(const wchar *a, const wchar *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
/* FNames are case-insensitive (HP3's level object is "myLevel", not "MyLevel") */
static wchar wlow(wchar c) { return (c >= 'A' && c <= 'Z') ? (wchar)(c + 32) : c; }
static int wieq(const wchar *a, const wchar *b) { while (*a && wlow(*a) == wlow(*b)) { a++; b++; } return wlow(*a) == wlow(*b); }
static void wcpy(wchar *d, const wchar *s, int max) { int i = 0; for (; s[i] && i < max - 1; i++) d[i] = s[i]; d[i] = 0; }

/* name of FName index into buf; tries shrinking reads so we never fail on a page boundary */
static int name_of(int idx, wchar *buf, int max) {
    u32 data = rd32((u32)gNames), num = rd32((u32)gNames + 4), e; int n;
    if (idx < 0 || (u32)idx >= num) return 0;
    e = rd32(data + idx * 4);
    if (!e) return 0;
    for (n = max - 1; n >= 8; n /= 2) if (rd(e + 0x0C, buf, n * 2)) { buf[n] = 0; return 1; }
    return 0;
}
/* FName index of a string (scan of the name table; names never move) */
static int name_index(const wchar *want) {
    u32 num = rd32((u32)gNames + 4), i; wchar b[64];
    for (i = 0; i < num; i++) if (name_of((int)i, b, 64) && wieq(b, want)) return (int)i;
    return -1;
}

/* walk every object, 1024 pointers per read, 8 bytes (Name, Class) per object */
typedef int (*visit_fn)(u32 obj, int name, u32 cls, void *ctx);   /* return 1 to stop */
static void for_each_object(visit_fn fn, void *ctx) {
    static u32 ptrs[1024];   /* only the worker thread uses this */
    u32 data = rd32((u32)gObjs), num = rd32((u32)gObjs + 4), i, j, chunk, nc[2];
    for (i = 0; i < num; i += 1024) {
        chunk = num - i < 1024 ? num - i : 1024;
        if (!rd(data + i * 4, ptrs, chunk * 4)) continue;
        for (j = 0; j < chunk; j++) {
            if (!ptrs[j] || !rd(ptrs[j] + 0x20, nc, 8)) continue;
            if (fn(ptrs[j], (int)nc[0], nc[1], ctx)) return;
        }
    }
}

static int N_Class = -1;
struct findcls { int name; u32 found; };
static int v_findcls(u32 o, int name, u32 cls, void *c) {
    struct findcls *f = c;
    if (name == f->name && cls && (int)rd32(cls + 0x20) == N_Class) { f->found = o; return 1; }
    return 0;
}
static u32 find_class(const wchar *cname) {
    struct findcls f; f.name = name_index(cname); f.found = 0;
    if (f.name >= 0) for_each_object(v_findcls, &f);
    return f.found;
}
struct findprop { int prop, cls; int off; };
static int v_findprop(u32 o, int name, u32 cls, void *c) {
    struct findprop *f = c; (void)cls;
    if (name == f->prop && (int)rd32(rd32(o + 0x18) + 0x20) == f->cls) { f->off = (int)rd32(o + 0x4C); return 1; }
    return 0;
}
/* byte offset of property `prop` declared in class `cls` (-1 if not found) */
static int prop_offset(const wchar *cls, const wchar *prop) {
    struct findprop f; f.prop = name_index(prop); f.cls = name_index(cls); f.off = -1;
    if (f.prop >= 0 && f.cls >= 0) for_each_object(v_findprop, &f);
    return f.off;
}
static int is_a_cls(u32 c, u32 want) { int guard = 64; while (c && guard--) { if (c == want) return 1; c = rd32(c + 0x2C); } return 0; }

/* one pass: every PlayerController instance (the live one is picked by the caller) */
struct scan { u32 clsPC, clsClass; u32 pcs[16]; int npc; u32 known[512]; int nknown; };
static int v_scan(u32 o, int name, u32 cls, void *c) {
    struct scan *s = c; int k; (void)name;
    if (!cls || cls == s->clsClass || !s->clsPC) return 0;
    for (k = 0; k < s->nknown; k++) if (s->known[k] == cls) return 0;   /* class already known not to match */
    if (is_a_cls(cls, s->clsPC)) { if (s->npc < 16) s->pcs[s->npc++] = o; return 0; }
    if (s->nknown < 512) s->known[s->nknown++] = cls;
    return 0;
}

/* LevelInfo.ID -> map file (taken from each map's LevelInfo0; IDs are case-insensitive) */
static const wchar *MAP_IDS[][2] = {
    {L"ent", L"Entry"}, {L"pre", L"HP_Preamble"}, {L"adv1", L"HP3_Adv1Express"}, {L"adv2", L"HP3_Adv2ExpPatronum"},
    {L"adv3", L"HP3_Adv3Library"}, {L"adv5shack", L"HP3_Adv5Shack"}, {L"adv6", L"HP3_Adv6Paddock"}, {L"bb", L"HP3_BeanBonus"},
    {L"bex", L"HP3_BuckyExecuted"}, {L"comc", L"HP3_COMC"}, {L"cr", L"HP3_Ch1CarpeRetractum"}, {L"crb", L"HP3_Ch1CarpeRetractumB"},
    {L"dl", L"HP3_Ch2Draconifors"}, {L"dlb", L"HP3_Ch2DraconiforsB"}, {L"gl", L"HP3_Ch3Glacius"}, {L"glb", L"HP3_Ch3GlaciusB"},
    {L"tower", L"HP3_DarkTower"}, {L"dembtl", L"HP3_DementorBattle"}, {L"dung", L"HP3_DungeonHUB"}, {L"rf", L"HP3_FA1Ron"},
    {L"hf", L"HP3_FA2Hermione"}, {L"hfe", L"HP3_FA3Harry"}, {L"hp3final", L"HP3_Final"}, {L"grounddada", L"HP3_GroundsDada"},
    {L"ghub", L"HP3_GroundsHub"}, {L"inf", L"HP3_Infirmary"}, {L"ihub", L"HP3_InsideHub"}, {L"ppb", L"HP3_PortraitPassword_B"},
    {L"ppc", L"HP3_PortraitPassword_C"}, {L"ppd", L"HP3_PortraitPassword_D"}, {L"ppe", L"HP3_PortraitPassword_E"},
    {L"ppf", L"HP3_PortraitPassword_F"}, {L"quid", L"HP3_Quidditch"}, {L"whompingwillow", L"HP3_WhompingWillow"},
};
static const wchar *map_for_id(const wchar *id) {
    unsigned k;
    for (k = 0; k < sizeof MAP_IDS / sizeof MAP_IDS[0]; k++) if (wieq(id, MAP_IDS[k][0])) return MAP_IDS[k][1];
    return 0;
}

static int read_fstring(u32 addr, wchar *buf, int max) {
    u32 p = rd32(addr); int n = (int)rd32(addr + 4);
    buf[0] = 0;
    if (!p || n <= 0) return 0;
    if (n > max) n = max;
    if (!rd(p, buf, n * 2)) { buf[0] = 0; return 0; }
    buf[n - 1] = 0; return 1;
}

/* ---- output ---- */
static wchar gLogPath[600];
static void log_line(const wchar *w) {
    char u8[1200]; u32 wr; SYSTEMTIME t; wchar line[700]; int n; HANDLE f;
    GetLocalTime(&t);
    wsprintfW(line, L"%04d-%02d-%02d %02d:%02d:%02d  %s\r\n", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, w);
    n = WideCharToMultiByte(65001, 0, line, -1, u8, sizeof u8, 0, 0);
    f = CreateFileW(gLogPath, 0x0004 /*FILE_APPEND_DATA*/, 1 | 2, 0, 4 /*OPEN_ALWAYS*/, 0x80, 0);
    if (f == (HANDLE)-1) return;
    if (n > 1) WriteFile(f, u8, n - 1, &wr, 0);
    CloseHandle(f);
}
static HWND gWnd; static wchar gOrigTitle[256];
static BOOL WINAPI enum_cb(HWND h, long lp) {
    u32 pid = 0; wchar t[8]; (void)lp;
    GetWindowThreadProcessId(h, &pid);
    if (pid == GetCurrentProcessId() && IsWindowVisible(h) && GetWindowTextW(h, t, 8) > 0) { gWnd = h; return 0; }
    return 1;
}
static void set_title(const wchar *s) {
    u32 res;
    if (!gWnd) { EnumWindows(enum_cb, 0); if (gWnd) GetWindowTextW(gWnd, gOrigTitle, 256); }
    if (gWnd) SendMessageTimeoutW(gWnd, WM_SETTEXT, 0, (long)s, SMTO_ABORTIFHUNG, 500, &res);
}

/* property offsets, looked up lazily (kwGame.u loads after the engine starts) */
static int offGS = -1, offPawn = -1, offLoc = -1, offLevel = -1, offXLevel = -1, offID = -1, offGame = -1;
static void lookup_offsets(void) {
    if (offPawn < 0)   offPawn   = prop_offset(L"Controller", L"Pawn");
    if (offLoc < 0)    offLoc    = prop_offset(L"Actor", L"Location");
    if (offLevel < 0)  offLevel  = prop_offset(L"Actor", L"Level");
    if (offXLevel < 0) offXLevel = prop_offset(L"Actor", L"XLevel");
    if (offID < 0)     offID     = prop_offset(L"LevelInfo", L"ID");
    if (offGame < 0)   offGame   = prop_offset(L"LevelInfo", L"Game");
    if (offGS < 0)     offGS     = prop_offset(L"KWGame", L"CurrentGameState");
}

static u32 WINAPI worker(void *arg) {
    HANDLE core; wchar map[64] = {0}, lastMap[64] = {0}, gs[32] = {0}, lastGs[32] = {0}, buf[400], id[32], pkg[64];
    int f11prev = 0, i, reported = 0; u32 tick = 0, pc = 0, pawn = 0;
    static struct scan sc;   /* 2 KB+, keep it off the stack */
    (void)arg;
    gProc = GetCurrentProcess();
    /* log path first, so every startup stage is visible even if a later one stalls */
    {   wchar docs[520];
        if (SHGetFolderPathW(0, 5 /*CSIDL_PERSONAL*/, 0, 0, docs) == 0)
            wsprintfW(gLogPath, L"%s\\Harry Potter and the Prisoner of Azkaban\\HP3_MapLog.txt", docs);
        else wcpy(gLogPath, L"HP3_MapLog.txt", 600);
    }
    log_line(L"---- HP3 MapTracker " MT_VERSION L" loaded ----");
    for (i = 0; !(core = GetModuleHandleW(L"Core.dll")); i++) {
        if (i == 20) log_line(L"waiting for Core.dll ...");
        Sleep(500);
    }
    gObjs  = (u32*)GetProcAddress(core, "?GObjObjects@UObject@@0V?$TArray@PAVUObject@@@@A");
    gNames = (u32*)GetProcAddress(core, "?Names@FName@@0V?$TArray@PAUFNameEntry@@@@A");
    wsprintfW(buf, L"Core.dll exports: GObjObjects %s, FName::Names %s", gObjs ? L"ok" : L"MISSING", gNames ? L"ok" : L"MISSING");
    log_line(buf);
    if (!gObjs || !gNames) return 1;
    for (i = 0;; i++) {
        Sleep(1000);
        if (rd32((u32)gObjs + 4) < 1000) continue;
        N_Class = name_index(L"Class");
        if (N_Class >= 0) break;
        if (i % 10 == 0) log_line(L"waiting for engine names ...");
    }
    for (;;) {
        /* F11 marker, polled fast */
        int down = (GetAsyncKeyState(VK_F11) & 0x8000) != 0;
        if (down && !f11prev && gWnd && GetForegroundWindow() == gWnd) {
            float loc[3] = {0, 0, 0};
            if (pawn && offLoc >= 0) rd(pawn + offLoc, loc, 12);
            wsprintfW(buf, L"MARK  map=%s  state=%s  pos=(%d, %d, %d)", map[0] ? map : L"?", gs[0] ? gs : L"?",
                      (int)loc[0], (int)loc[1], (int)loc[2]);
            log_line(buf);
        }
        f11prev = down;
        if (tick++ % 20 == 0) {   /* about once a second */
            u32 li = 0, lvl, game;
            if (!sc.clsPC) { sc.clsPC = find_class(L"PlayerController"); sc.clsClass = sc.clsPC ? rd32(sc.clsPC + 0x24) : 0; }
            lookup_offsets();
            if (!reported && offGS >= 0) {
                wsprintfW(buf, L"ready: offsets pawn=%d loc=%d level=%d xlevel=%d id=%d game=%d gamestate=%d",
                          offPawn, offLoc, offLevel, offXLevel, offID, offGame, offGS);
                log_line(buf); reported = 1;
            }
            /* the live controller is the one with a pawn (menu/old-level controllers have none); newest wins */
            sc.npc = 0; sc.nknown = 0; pc = 0; pawn = 0;
            if (sc.clsPC && offPawn >= 0) {
                for_each_object(v_scan, &sc);
                for (i = 0; i < sc.npc; i++) { u32 p = rd32(sc.pcs[i] + offPawn); if (p) { pc = sc.pcs[i]; pawn = p; } }
            }
            map[0] = 0; gs[0] = 0; id[0] = 0; pkg[0] = 0;
            if (pc && offLevel >= 0) li = rd32(pc + offLevel);
            if (li && offID >= 0) read_fstring(li + offID, id, 32);
            if (pc && offXLevel >= 0 && (lvl = rd32(pc + offXLevel)) != 0) name_of((int)rd32(rd32(lvl + 0x18) + 0x20), pkg, 64);
            if (id[0] && map_for_id(id)) wcpy(map, map_for_id(id), 64);
            else if (pkg[0]) wcpy(map, pkg, 64);
            if (li && offGame >= 0 && offGS >= 0 && (game = rd32(li + offGame)) != 0) read_fstring(game + offGS, gs, 32);
            if (!weq(map, lastMap) || !weq(gs, lastGs)) {
                wsprintfW(buf, L"MAP %s  state=%s  (id=%s package=%s)", map[0] ? map : L"(none)", gs[0] ? gs : L"(none)",
                          id[0] ? id : L"-", pkg[0] ? pkg : L"-");
                log_line(buf);
                wcpy(lastMap, map, 64); wcpy(lastGs, gs, 32);
                gWnd = 0;  /* re-find the window after loads */
                wsprintfW(buf, L"HP3  |  %s  |  %s  |  F11 = mark", map[0] ? map : L"...", gs[0] ? gs : L"-");
                set_title(buf);
            }
        }
        Sleep(50);
    }
}

BOOL WINAPI DllMain(HANDLE inst, u32 reason, void *res) {
    (void)res;
    if (reason == 1) { DisableThreadLibraryCalls(inst); CreateThread(0, 0, worker, 0, 0, 0); }
    return 1;
}
