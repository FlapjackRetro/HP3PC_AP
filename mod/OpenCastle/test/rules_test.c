/* host test for rules.h: gcc -fshort-wchar -o rules_test rules_test.c && ./rules_test */
#include <stdio.h>
typedef unsigned short wchar; typedef unsigned int u32;
static void *gProc; static u32 *gObjs, *gNames;
static int ReadProcessMemory(void *a, const void *b, void *c, u32 d, u32 *e) { (void)a; (void)b; (void)c; (void)d; (void)e; return 0; }
#include "../../common/ue2read.h"
#include "../rules.h"
static void w2a(const wchar *w, char *a) { while (*w) *a++ = (char)*w++; *a = 0; }
static int fails;
static void t(const wchar *src, const wchar *in, const wchar *want) {
    wchar out[600], why[128]; char a[600], b[600], c[600], r[128];
    int ch = oc_rewrite(src, in, out, 600, why, 128);
    w2a(in, a); w2a(ch ? out : in, b); w2a(want ? want : in, c); w2a(why, r);
    if (wieq(ch ? out : in, want ? want : in) && (want != 0) == (ch != 0)) printf("PASS  %s -> %s  (%s)\n", a, b, r);
    else { printf("FAIL  %s -> %s, want %s\n", a, b, c); fails++; }
}
int main(void) {
    t(L"HP_Preamble", L"HP3_Adv1Express?GameState=GSTATE000", L"HP3_InsideHub?GameState=GState090");
    t(L"", L"..\\Maps\\HP3_Adv1Express.unr", L"HP3_InsideHub?GameState=GState090");
    t(L"HP3_InsideHub", L"HP3_Adv1Express?GameState=GState090", 0);                 /* via the 7th-floor door: unchanged */
    t(L"HP3_InsideHub", L"HP3_Ch1CarpeRetractum?quiet?GameState=GState090", L"HP3_FA1Ron?quiet?GameState=GState090");
    t(L"HP3_InsideHub", L"HP3_Ch1CarpeRetractum.unr?quiet?GameState=GState090", 0);  /* side room: unchanged */
    t(L"HP3_InsideHub", L"HP3_Ch2Draconifors?quiet?GameState=GState065", L"HP3_FA2Hermione?quiet?GameState=GState090");
    t(L"HP3_InsideHub", L"HP3_Ch3Glacius?quiet?GameState=GState090", L"HP3_FA3Harry?quiet?GameState=GState090");
    t(L"HP3_InsideHub", L"HP3_Ch2Draconifors.unr?GameState=GState060", L"HP3_Ch2Draconifors.unr?GameState=GState090");
    t(L"HP3_Adv5Shack", L"HP3_WhompingWillow?quiet?GameState=GState085", L"HP3_DungeonHUB?quiet?GameState=GState090");
    t(L"HP3_Adv6Paddock", L"HP3_DementorBattle?GameState=GState090", L"HP3_InsideHub?GameState=GState090");
    t(L"HP3_Adv1Express", L"HP3_GroundsDADA?GameState=GState000", L"HP3_InsideHub?GameState=GState090");
    t(L"HP3_Adv2ExpPatronum", L"HP3_Adv3Library?GameState=GState070", L"HP3_InsideHub?GameState=GState090");
    t(L"HP3_FA1Ron", L"HP3_FA2Hermione?GameState=GState100", L"HP3_InsideHub?GameState=GState090");
    t(L"HP3_FA3Harry", L"HP3_GroundsHub?quiet?GameState=GState100", L"HP3_InsideHub?quiet?GameState=GState090");
    t(L"HP3_COMC", L"HP3_GroundsHub?quiet?GameState=GState090", 0);                  /* already fine */
    t(L"HP3_GroundsHub", L"HP3_COMC?quiet", L"HP3_COMC?quiet?GameState=GState090");
    t(L"HP3_InsideHub", L"HP3_GroundsHub?GameState=GState040#Start", L"HP3_GroundsHub?GameState=GState090#Start");
    t(L"HP3_InsideHub", L"Save100?load=1", 0);                                        /* save loads untouched */
    t(L"HP3_InsideHub", L"HP_Preamble?quiet", 0);
    t(L"HP3_InsideHub", L"HP3_InsideHub?SaveGame=3", 0);
    printf(fails ? "\n%d FAILED\n" : "\nALL PASSED\n", fails);
    return fails != 0;
}
