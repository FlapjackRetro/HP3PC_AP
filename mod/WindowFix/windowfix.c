/*
 * HP3 WindowFix - Ultimate ASI Loader plugin (plugins\HP3WindowFix.asi)
 *
 * 1. Remembers fullscreen/windowed between launches. The game reads
 *    [WinDrv.WindowsClient] StartupFullscreen from hppoa.ini at startup but Alt+Enter never
 *    writes it back. This plugin watches the game window and writes the current mode to that
 *    key whenever it changes, and once more on exit (after the engine has saved its config).
 * 2. Remembers where the window was. When the window is moved, its position is saved to
 *    HP3_WindowFix.ini (next to hppoa.ini; a separate file because the engine rewrites hppoa.ini
 *    from its own copy on exit). Whenever the game becomes windowed (at startup or after
 *    Alt+Enter) the window goes back there, or is centered if there's no saved position or it's
 *    no longer on any monitor.
 *
 * Fallback: if the ini says windowed but the game still comes up fullscreen, it presses
 * Alt+Enter once for you.
 *
 * It never touches game memory; it only uses window APIs and the ini file.
 * No CRT; built with clang + lld-link (see build.sh).
 */
typedef unsigned short wchar;
typedef unsigned int   u32;
typedef int            BOOL;
typedef void*          HANDLE;
typedef void*          HWND;
#define WINAPI __stdcall
#define IMP __declspec(dllimport)

typedef struct { long left, top, right, bottom; } RECT;
typedef struct { u32 cbSize; RECT rcMonitor, rcWork; u32 dwFlags; } MONITORINFO;
typedef struct { unsigned short wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMs; } SYSTEMTIME;

IMP HANDLE WINAPI CreateThread(void*, u32, u32 (WINAPI*)(void*), void*, u32, u32*);
IMP void   WINAPI Sleep(u32);
IMP u32    WINAPI GetCurrentProcessId(void);
IMP BOOL   WINAPI DisableThreadLibraryCalls(HANDLE);
IMP u32    WINAPI GetFileAttributesW(const wchar*);
IMP u32    WINAPI GetModuleFileNameW(HANDLE, wchar*, u32);
IMP BOOL   WINAPI WritePrivateProfileStringW(const wchar*, const wchar*, const wchar*, const wchar*);
IMP u32    WINAPI GetPrivateProfileIntW(const wchar*, const wchar*, int, const wchar*);
IMP u32    WINAPI GetPrivateProfileStringW(const wchar*, const wchar*, const wchar*, wchar*, u32, const wchar*);
IMP HANDLE WINAPI CreateFileW(const wchar*, u32, u32, void*, u32, u32, HANDLE);
IMP BOOL   WINAPI WriteFile(HANDLE, const void*, u32, u32*, void*);
IMP BOOL   WINAPI CloseHandle(HANDLE);
IMP void   WINAPI GetLocalTime(SYSTEMTIME*);
IMP int    WINAPI WideCharToMultiByte(u32, u32, const wchar*, int, char*, int, const char*, BOOL*);
IMP BOOL   WINAPI EnumWindows(BOOL (WINAPI*)(HWND, long), long);
IMP u32    WINAPI GetWindowThreadProcessId(HWND, u32*);
IMP BOOL   WINAPI IsWindowVisible(HWND);
IMP BOOL   WINAPI IsWindow(HWND);
IMP BOOL   WINAPI IsIconic(HWND);
IMP BOOL   WINAPI GetWindowRect(HWND, RECT*);
IMP long   WINAPI GetWindowLongW(HWND, int);
IMP BOOL   WINAPI SetWindowPos(HWND, HWND, int, int, int, int, u32);
IMP HANDLE WINAPI MonitorFromWindow(HWND, u32);
IMP HANDLE WINAPI MonitorFromRect(const RECT*, u32);
IMP BOOL   WINAPI GetMonitorInfoW(HANDLE, MONITORINFO*);
IMP HWND   WINAPI GetForegroundWindow(void);
IMP short  WINAPI GetAsyncKeyState(int);
IMP void   WINAPI keybd_event(unsigned char, unsigned char, u32, u32);
IMP int    __cdecl wsprintfW(wchar*, const wchar*, ...);
IMP long   WINAPI SHGetFolderPathW(HWND, int, HANDLE, u32, wchar*);

#define GWL_STYLE (-16)
#define WS_CAPTION 0x00C00000L
#define SWP_NOSIZE 0x0001
#define SWP_NOZORDER 0x0004
#define SWP_NOACTIVATE 0x0010
#define MONITOR_DEFAULTTONEAREST 2
#define VK_RETURN 0x0D
#define VK_MENU 0x12
#define KEYEVENTF_KEYUP 2
#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFFu
#define MONITOR_DEFAULTTONULL 0
#define NOPOS (-32000)
#define WF_VERSION L"v3"

int _fltused = 0;
void *memset(void *d, int c, unsigned n) { volatile unsigned char *p = (volatile unsigned char *)d; while (n--) *p++ = (unsigned char)c; return d; }

static wchar gIni[600], gLog[600], gPosIni[600];
static volatile int gMode = -1;   /* 1 = fullscreen, 0 = windowed, -1 = unknown */

static void log_line(const wchar *msg) {
    static wchar line[700]; static char u8[1400]; SYSTEMTIME t; u32 wr; int n; HANDLE f;
    GetLocalTime(&t);
    wsprintfW(line, L"%04d-%02d-%02d %02d:%02d:%02d  %s\r\n", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, msg);
    n = WideCharToMultiByte(65001, 0, line, -1, u8, sizeof u8, 0, 0);
    f = CreateFileW(gLog, 4 /*FILE_APPEND_DATA*/, 3, 0, 4 /*OPEN_ALWAYS*/, 0x80, 0);
    if (f == (HANDLE)-1) return;
    if (n > 1) WriteFile(f, u8, n - 1, &wr, 0);
    CloseHandle(f);
}

/* the game's main window: the largest visible top-level window of this process */
static HWND gBest; static long gBestArea;
static BOOL WINAPI enum_cb(HWND h, long lp) {
    u32 pid = 0; RECT r; long a; (void)lp;
    GetWindowThreadProcessId(h, &pid);
    if (pid != GetCurrentProcessId() || !IsWindowVisible(h) || !GetWindowRect(h, &r)) return 1;
    a = (r.right - r.left) * (r.bottom - r.top);
    if (a > gBestArea) { gBestArea = a; gBest = h; }
    return 1;
}
static HWND find_game_window(void) { gBest = 0; gBestArea = 0; EnumWindows(enum_cb, 0); return gBest; }

/* fullscreen = no title bar and covering the whole monitor */
static int is_fullscreen(HWND h) {
    RECT r; MONITORINFO mi; HANDLE mon;
    if (GetWindowLongW(h, GWL_STYLE) & WS_CAPTION) return 0;
    mon = MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST);
    mi.cbSize = sizeof mi;
    if (!GetWindowRect(h, &r) || !GetMonitorInfoW(mon, &mi)) return 0;
    return r.left <= mi.rcMonitor.left && r.top <= mi.rcMonitor.top &&
           r.right >= mi.rcMonitor.right && r.bottom >= mi.rcMonitor.bottom;
}

static void save_mode(int fs) {
    if (gIni[0]) WritePrivateProfileStringW(L"WinDrv.WindowsClient", L"StartupFullscreen", fs ? L"True" : L"False", gIni);
}

/* saved window position, in our own ini */
static void save_pos(long x, long y) {
    wchar v[16];
    wsprintfW(v, L"%d", (int)x); WritePrivateProfileStringW(L"Window", L"X", v, gPosIni);
    wsprintfW(v, L"%d", (int)y); WritePrivateProfileStringW(L"Window", L"Y", v, gPosIni);
}
/* move to the saved position if its title bar would be on some monitor; returns 0 if not */
static int restore_pos(HWND h) {
    RECT r, bar; long x, y; wchar buf[160];
    x = (int)GetPrivateProfileIntW(L"Window", L"X", NOPOS, gPosIni);
    y = (int)GetPrivateProfileIntW(L"Window", L"Y", NOPOS, gPosIni);
    if (x == NOPOS || y == NOPOS || !GetWindowRect(h, &r)) return 0;
    bar.left = x; bar.top = y; bar.right = x + (r.right - r.left); bar.bottom = y + 30;
    if (!MonitorFromRect(&bar, MONITOR_DEFAULTTONULL)) return 0;
    SetWindowPos(h, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    wsprintfW(buf, L"restored %dx%d window to (%d, %d)", (int)(r.right - r.left), (int)(r.bottom - r.top), (int)x, (int)y);
    log_line(buf);
    return 1;
}

/* center on the monitor's work area; if the window is taller/wider than it, keep the title bar on screen */
static void center(HWND h) {
    RECT r; MONITORINFO mi; long w, ht, x, y; wchar buf[160];
    mi.cbSize = sizeof mi;
    if (!GetWindowRect(h, &r) || !GetMonitorInfoW(MonitorFromWindow(h, MONITOR_DEFAULTTONEAREST), &mi)) return;
    w = r.right - r.left; ht = r.bottom - r.top;
    x = mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - w) / 2;
    y = mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) - ht) / 2;
    if (x < mi.rcWork.left) x = mi.rcWork.left;
    if (y < mi.rcWork.top)  y = mi.rcWork.top;
    SetWindowPos(h, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    wsprintfW(buf, L"centered %dx%d window at (%d, %d)", (int)w, (int)ht, (int)x, (int)y);
    log_line(buf);
}

static void press_alt_enter(void) {
    keybd_event(VK_MENU, 0x38, 0, 0);
    keybd_event(VK_RETURN, 0x1C, 0, 0);
    keybd_event(VK_RETURN, 0x1C, KEYEVENTF_KEYUP, 0);
    keybd_event(VK_MENU, 0x38, KEYEVENTF_KEYUP, 0);
}

static int same_rect(const RECT *a, const RECT *b) {
    return a->left == b->left && a->top == b->top && a->right == b->right && a->bottom == b->bottom;
}

static u32 WINAPI worker(void *arg) {
    HWND h = 0; RECT r, last = {0, 0, 0, 0}; int stable = 0, placed = 0, dragged = 0, fs, wantWindowed, fallbackDone = 0, centerPending = 0;
    long placed_x = 0, placed_y = 0, placed_w = 0, placed_h = 0; u32 ticks = 0; wchar buf[700], val[16];
    (void)arg;

    /* the game keeps its ini in Documents\<game>\hppoa.ini; fall back to system\hppoa.ini next to the exe */
    {   wchar docs[520], *p, *slash = 0;
        if (SHGetFolderPathW(0, 5 /*CSIDL_PERSONAL*/, 0, 0, docs) == 0) {
            wsprintfW(gLog, L"%s\\Harry Potter and the Prisoner of Azkaban\\HP3_WindowFix.log", docs);
            wsprintfW(gIni, L"%s\\Harry Potter and the Prisoner of Azkaban\\hppoa.ini", docs);
            wsprintfW(gPosIni, L"%s\\Harry Potter and the Prisoner of Azkaban\\HP3_WindowFix.ini", docs);
        }
        if (!gIni[0] || GetFileAttributesW(gIni) == INVALID_FILE_ATTRIBUTES) {
            GetModuleFileNameW(0, gIni, 520);
            for (p = gIni; *p; p++) if (*p == '\\') slash = p;
            if (slash) wsprintfW(slash + 1, L"hppoa.ini");
            if (!gLog[0]) wsprintfW(gLog, L"HP3_WindowFix.log");
        }
        if (!gPosIni[0]) {   /* no Documents folder: keep it next to hppoa.ini */
            slash = 0; wsprintfW(gPosIni, L"%s", gIni);
            for (p = gPosIni; *p; p++) if (*p == '\\') slash = p;
            if (slash) wsprintfW(slash + 1, L"HP3_WindowFix.ini");
        }
    }
    GetPrivateProfileStringW(L"WinDrv.WindowsClient", L"StartupFullscreen", L"True", val, 16, gIni);
    wantWindowed = (val[0] == 'F' || val[0] == 'f' || val[0] == '0');
    wsprintfW(buf, L"---- HP3 WindowFix " WF_VERSION L" loaded: StartupFullscreen=%s in %s ----", val, gIni);
    log_line(buf);

    for (;; Sleep(250), ticks++) {
        if (!h || !IsWindow(h)) { h = find_game_window(); stable = 0; if (!h) continue; }
        if (IsIconic(h) || !GetWindowRect(h, &r)) continue;
        /* wait until the window has stopped changing for ~0.75 s (mode switches resize it in steps) */
        if (!same_rect(&r, &last)) {
            last = r; stable = 0;
            if ((GetAsyncKeyState(1) | GetAsyncKeyState(2)) & 0x8000) dragged = 1;   /* mouse held while it moved */
            continue;
        }
        if (++stable < 3) continue;

        fs = is_fullscreen(h);
        if (fs != gMode) {
            int first = (gMode < 0);
            gMode = fs;
            save_mode(fs);
            wsprintfW(buf, L"%s: %s  (saved StartupFullscreen=%s)", first ? L"started" : L"switched",
                      fs ? L"fullscreen" : L"windowed", fs ? L"True" : L"False");
            log_line(buf);
            if (!fs) centerPending = 1;
            placed = 0;
            /* the ini asked for windowed but the game came up fullscreen anyway: toggle once */
            if (first && fs && wantWindowed && !fallbackDone && GetForegroundWindow() == h) {
                fallbackDone = 1;
                log_line(L"ini says windowed but the game started fullscreen; pressing Alt+Enter");
                press_alt_enter();
            }
        }
        if (fs) continue;
        /* The game resizes the window after creating it (640x480 first, then the configured size) and
           moves it to its own default spot when it does. So any size change is the game's doing: put the
           window back. A move without a resize is the user dragging it: remember that. */
        if (centerPending || !placed || r.right - r.left != placed_w || r.bottom - r.top != placed_h) {
            centerPending = 0;
            if (!restore_pos(h)) center(h);
            GetWindowRect(h, &r);
            placed = 1; placed_x = r.left; placed_y = r.top; placed_w = r.right - r.left; placed_h = r.bottom - r.top;
            last = r; stable = 0; dragged = 0;
            continue;
        }
        if (r.left != placed_x || r.top != placed_y) {
            if (!dragged) {   /* the game moved it, not the user: put it back */
                wsprintfW(buf, L"game moved the window to (%d, %d); moving it back", (int)r.left, (int)r.top);
                log_line(buf);
                SetWindowPos(h, 0, placed_x, placed_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                last.left = -99999; stable = 0;
                continue;
            }
            dragged = 0;
            placed_x = r.left; placed_y = r.top;
            save_pos(r.left, r.top);
            wsprintfW(buf, L"window moved to (%d, %d), saved", (int)r.left, (int)r.top);
            log_line(buf);
        }
    }
}

BOOL WINAPI DllMain(HANDLE inst, u32 reason, void *res) {
    (void)res;
    if (reason == 1) { DisableThreadLibraryCalls(inst); CreateThread(0, 0, worker, 0, 0, 0); }
    /* on exit the engine rewrites hppoa.ini from its own copy, so write the mode again afterwards */
    else if (reason == 0 && gMode >= 0) save_mode(gMode);
    return 1;
}
