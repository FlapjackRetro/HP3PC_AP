/* Host-side test: fake 32-bit address space laid out like HP3's engine, MapTracker logic compiled against it. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#define __declspec(x)
#define __stdcall
#define __cdecl
#define wsprintfW wsprintfW_unused
#define main_dll DllMain
static unsigned char MEM[1 << 22]; static unsigned top = 0x1000;
static unsigned alloc(unsigned n) { unsigned a = top; top += (n + 15) & ~15u; return a; }
static void put32(unsigned a, unsigned v) { memcpy(MEM + a, &v, 4); }
#define memset mt_memset
#include "../maptracker.c"
#undef memset
#undef wsprintfW
/* imported functions used by the code under test */
BOOL ReadProcessMemory(HANDLE h, const void *a, void *o, u32 n, u32 *got) { (void)h; uintptr_t ad=(uintptr_t)a; if (!ad || ad + n > sizeof MEM) { *got = 0; return 0; } memcpy(o, MEM + ad, n); *got = n; return 1; }
HANDLE GetCurrentProcess(void){return 0;} HANDLE GetModuleHandleW(const wchar*w){(void)w;return 0;} void*GetProcAddress(HANDLE h,const char*c){(void)h;(void)c;return 0;}
HANDLE CreateThread(void*a,u32 b,u32(*c)(void*),void*d,u32 e,u32*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return 0;} void Sleep(u32 x){(void)x;}
u32 GetCurrentProcessId(void){return 1;} BOOL DisableThreadLibraryCalls(HANDLE h){(void)h;return 1;}
HANDLE CreateFileW(const wchar*a,u32 b,u32 c,void*d,u32 e,u32 f,HANDLE g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;return (HANDLE)-1;}
BOOL WriteFile(HANDLE a,const void*b,u32 c,u32*d,void*e){(void)a;(void)b;(void)c;(void)d;(void)e;return 1;} BOOL CloseHandle(HANDLE a){(void)a;return 1;}
void GetLocalTime(SYSTEMTIME*t){memset(t,0,sizeof*t);} int WideCharToMultiByte(u32 a,u32 b,const wchar*c,int d,char*e,int f,const char*g,BOOL*h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return 0;}
BOOL EnumWindows(BOOL(*a)(HWND,long),long b){(void)a;(void)b;return 1;} u32 GetWindowThreadProcessId(HWND a,u32*b){(void)a;(void)b;return 0;} BOOL IsWindowVisible(HWND a){(void)a;return 0;}
int GetWindowTextW(HWND a,wchar*b,int c){(void)a;(void)b;(void)c;return 0;} long SendMessageTimeoutW(HWND a,u32 b,u32 c,long d,u32 e,u32 f,u32*g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;return 0;}
HWND GetForegroundWindow(void){return 0;} short GetAsyncKeyState(int k){(void)k;return 0;} int wsprintfW(wchar*a,const wchar*b,...){(void)a;(void)b;return 0;} long SHGetFolderPathW(HWND a,int b,HANDLE c,u32 d,wchar*e){(void)a;(void)b;(void)c;(void)d;(void)e;return 1;}

/* ---- build fake engine state ---- */
static unsigned namesArr, nNames, objsArr, nObjs;
static int addname(const char *s) {
    unsigned e = alloc(0x0C + 2 * 64); unsigned i;
    for (i = 0; s[i]; i++) { MEM[e + 0x0C + 2*i] = (unsigned char)s[i]; MEM[e + 0x0C + 2*i + 1] = 0; }
    put32(namesArr + 4 * nNames, e); return (int)nNames++;
}
static unsigned mkobj(int name, unsigned cls, unsigned outer) {
    unsigned o = alloc(0x100); put32(o + 0x18, outer); put32(o + 0x20, (unsigned)name); put32(o + 0x24, cls);
    put32(objsArr + 4 * nObjs++, o); return o;
}
static unsigned wstr(const char *s) { unsigned a = alloc(64), i; for (i = 0; s[i]; i++) MEM[a + 2*i] = (unsigned char)s[i]; return a; }
#define CHECK(c, msg) do { if (c) printf("PASS  %s\n", msg); else { printf("FAIL  %s\n", msg); fails++; } } while (0)
int main(void) {
    int fails = 0; unsigned gNamesVar = alloc(12), gObjsVar = alloc(12);
    namesArr = alloc(4 * 4096); objsArr = alloc(4 * 3000);
    put32(gNamesVar, namesArr); put32(gObjsVar, objsArr);
    gNames = (u32*)(uintptr_t)gNamesVar; gObjs = (u32*)(uintptr_t)gObjsVar;
    int nNone = addname("None"), nClass = addname("Class"), nObject = addname("Object"), nActor = addname("Actor"),
        nInfo = addname("Info"), nGameInfo = addname("GameInfo"), nKWGame = addname("KWGame"), nHGame = addname("HGame"),
        nController = addname("Controller"), nPC = addname("PlayerController"), nHPHC = addname("HPHeroController"),
        nLevel = addname("Level"), nMyLevel = addname("myLevel"), nEntry = addname("Entry"),
        nInside = addname("HP3_InsideHub"), nGrounds = addname("HP3_GroundsHub"), nCGS = addname("CurrentGameState"),
        nPawn = addname("Pawn"), nLoc = addname("Location"), nPkg = addname("Package"), nStrProp = addname("StrProperty"),
        nHGame0 = addname("HGame0"), nPC0 = addname("HPHeroController0");
    (void)nNone;
    /* classes: Class object is its own class */
    unsigned cClass = mkobj(nClass, 0, 0); put32(cClass + 0x24, cClass);
    unsigned cObject = mkobj(nObject, cClass, 0), cActor = mkobj(nActor, cClass, 0), cInfo = mkobj(nInfo, cClass, 0),
             cGameInfo = mkobj(nGameInfo, cClass, 0), cKWGame = mkobj(nKWGame, cClass, 0), cHGame = mkobj(nHGame, cClass, 0),
             cController = mkobj(nController, cClass, 0), cPC = mkobj(nPC, cClass, 0), cHPHC = mkobj(nHPHC, cClass, 0),
             cLevel = mkobj(nLevel, cClass, 0), cPkg = mkobj(nPkg, cClass, 0), cStr = mkobj(nStrProp, cClass, 0);
    put32(cActor+0x2C,cObject); put32(cInfo+0x2C,cActor); put32(cGameInfo+0x2C,cInfo); put32(cKWGame+0x2C,cGameInfo);
    put32(cHGame+0x2C,cKWGame); put32(cController+0x2C,cActor); put32(cPC+0x2C,cController); put32(cHPHC+0x2C,cPC);
    /* properties (outer = declaring class, offset at +0x4C); decoys with same names elsewhere */
    unsigned pDecoy = mkobj(nPawn, cStr, cHGame); put32(pDecoy + 0x4C, 999);
    unsigned pCGS = mkobj(nCGS, cStr, cKWGame); put32(pCGS + 0x4C, 0x3A0);
    unsigned pPawn = mkobj(nPawn, cStr, cController); put32(pPawn + 0x4C, 0x2B0);
    unsigned pLoc = mkobj(nLoc, cStr, cActor); put32(pLoc + 0x4C, 0x150);
    /* packages + levels */
    unsigned kEntry = mkobj(nEntry, cPkg, 0), kInside = mkobj(nInside, cPkg, 0);
    mkobj(nMyLevel, cLevel, kEntry); unsigned lvInside = mkobj(nMyLevel, cLevel, kInside); (void)lvInside;
    /* instances: HGame0 with FString state; controller -> pawn with location */
    unsigned game = mkobj(nHGame0, cHGame, kInside);
    unsigned st = wstr("GState070"); put32(game + 0x3A0, st); put32(game + 0x3A4, 10); put32(game + 0x3A8, 10);
    unsigned pawn = alloc(0x400); float L[3] = {1234.5f, -567.0f, 89.9f}; memcpy(MEM + pawn + 0x150, L, 12);
    unsigned pc = mkobj(nPC0, cHPHC, kInside); put32(pc + 0x2B0, pawn);
    put32(objsArr + 4 * nObjs++, 0); /* a null slot */
    put32(gObjsVar + 4, nObjs); put32(gNamesVar + 4, nNames);

    N_Class = name_index(L"Class");
    CHECK(N_Class == nClass, "name_index finds Class");
    CHECK(name_index(L"MYLEVEL") == nMyLevel, "name_index is case-insensitive (myLevel)");
    CHECK(find_class(L"KWGame") == cKWGame && find_class(L"PlayerController") == cPC, "find_class returns class objects");
    CHECK(find_class(L"HGame0") == 0, "find_class ignores non-class objects");
    CHECK(prop_offset(L"KWGame", L"CurrentGameState") == 0x3A0, "prop_offset CurrentGameState");
    CHECK(prop_offset(L"Controller", L"Pawn") == 0x2B0, "prop_offset ignores same-named property in another class");
    CHECK(prop_offset(L"Actor", L"Location") == 0x150, "prop_offset Location");

    /* live chain: PC.Level -> LevelInfo (ID, Game) ; PC.XLevel -> Level -> package */
    int nLevelInfo = addname("LevelInfo"), nID = addname("ID"), nGame = addname("Game"), nLevelP = nLevel,
        nXLevel = addname("XLevel"), nSave = addname("Save100"), nPC1 = addname("MenuController0");
    unsigned cLI = mkobj(nLevelInfo, cClass, 0); put32(cLI + 0x2C, cInfo);
    unsigned pID = mkobj(nID, cStr, cLI); put32(pID + 0x4C, 0x500);
    unsigned pGame = mkobj(nGame, cStr, cLI); put32(pGame + 0x4C, 0x504);
    unsigned pLevel = mkobj(nLevelP, cStr, cActor); put32(pLevel + 0x4C, 0x9C);
    unsigned pXL = mkobj(nXLevel, cStr, cActor); put32(pXL + 0x4C, 0xA0);
    unsigned kSave = mkobj(nSave, cPkg, 0), lvSave = mkobj(nMyLevel, cLevel, kSave);
    unsigned li = mkobj(nLevelInfo, cLI, kSave); unsigned idstr = wstr("adv1");
    put32(li + 0x500, idstr); put32(li + 0x504, game); put32(li + 0x508, 5);  /* FString ID {data,num=5} then Game */
    put32(li + 0x504 - 4 + 4, 5); /* num already set via 0x504? keep layout explicit below */
    /* explicit layout: ID FString at 0x500 {ptr, num, max}; Game pointer at 0x510 */
    put32(li + 0x500, idstr); put32(li + 0x504, 5); put32(li + 0x508, 5); put32(li + 0x510, game); put32(pGame + 0x4C, 0x510);
    put32(pc + 0x9C, li); put32(pc + 0xA0, lvSave);
    unsigned menuPC = mkobj(nPC1, cHPHC, kInside); put32(menuPC + 0x2B0, 0);   /* stale controller without a pawn */
    put32(gObjsVar + 4, nObjs); put32(gNamesVar + 4, nNames);
    lookup_offsets();
    CHECK(offLevel == 0x9C && offXLevel == 0xA0 && offID == 0x500 && offGame == 0x510 && offGS == 0x3A0, "lazy offset lookup");
    static struct scan sc; memset(&sc, 0, sizeof sc); sc.clsPC = cPC; sc.clsClass = cClass;
    for_each_object(v_scan, &sc);
    unsigned live = 0; int i; for (i = 0; i < sc.npc; i++) if (rd32(sc.pcs[i] + offPawn)) live = sc.pcs[i];
    CHECK(sc.npc == 2 && live == pc, "picks the controller that has a pawn");
    wchar id[32], gs[32], pk[64];
    read_fstring(rd32(live + offLevel) + offID, id, 32);
    CHECK(weq(id, L"adv1") && weq(map_for_id(id), L"HP3_Adv1Express"), "LevelInfo.ID maps to the map name");
    CHECK(map_for_id(L"ADV5SHACK") && weq(map_for_id(L"ADV5SHACK"), L"HP3_Adv5Shack"), "ID lookup is case-insensitive");
    name_of((int)rd32(rd32(rd32(live + offXLevel) + 0x18) + 0x20), pk, 64);
    CHECK(weq(pk, L"Save100"), "XLevel package is the save (why the ID is needed)");
    read_fstring(rd32(rd32(live + offLevel) + offGame) + offGS, gs, 32);
    CHECK(weq(gs, L"GState070"), "Level.Game.CurrentGameState");
    float got[3]; rd(rd32(live + offPawn) + offLoc, got, 12);
    CHECK((int)got[0] == 1234 && (int)got[1] == -567 && (int)got[2] == 89, "reads pawn location");
    /* name read near the end of mapped memory must still succeed (shrinking reads) */
    unsigned edge = sizeof MEM - (0x0C + 2 * 20); { unsigned k; const char *s = "EdgeName"; memset(MEM + edge, 0, 0x0C + 40);
      for (k = 0; s[k]; k++) MEM[edge + 0x0C + 2*k] = (unsigned char)s[k]; }
    put32(namesArr + 4 * nNames, edge); put32(gNamesVar + 4, nNames + 1); wchar e2[64];
    CHECK(name_of((int)nNames, e2, 64) && weq(e2, L"EdgeName"), "name read at a memory boundary");
    printf(fails ? "\n%d FAILED\n" : "\nALL PASSED\n", fails);
    return fails != 0;
}
