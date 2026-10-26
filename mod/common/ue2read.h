/*
 * Read-only helpers for HP3's UE2 object system, shared by the HP3 plugins.
 * Every read goes through ReadProcessMemory on our own process, so a stale pointer fails instead of crashing.
 *
 * Engine layout (verified from Core.dll exports of the retail HP3 build):
 *   UObject: +0x18 Outer, +0x20 Name (FName index), +0x24 Class
 *   UStruct: +0x2C SuperField          FNameEntry: +0x0C wide-char name
 *   UProperty: +0x4C Offset            FString: { wchar_t* Data; int Num; int Max; }
 *
 * The including file provides: wchar, u32, gProc, gObjs, gNames, ReadProcessMemory.
 */
static int rd(u32 addr, void *out, u32 n) { u32 got = 0; return addr && ReadProcessMemory(gProc, (void*)addr, out, n, &got) && got == n; }
static u32 rd32(u32 addr) { u32 v = 0; rd(addr, &v, 4); return v; }

static int weq(const wchar *a, const wchar *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
/* FNames are case-insensitive (HP3's level object is "myLevel", not "MyLevel") */
static wchar wlow(wchar c) { return (c >= 'A' && c <= 'Z') ? (wchar)(c + 32) : c; }
static int wieq(const wchar *a, const wchar *b) { while (*a && wlow(*a) == wlow(*b)) { a++; b++; } return wlow(*a) == wlow(*b); }
static void wcpy(wchar *d, const wchar *s, int max) { int i = 0; for (; s[i] && i < max - 1; i++) d[i] = s[i]; d[i] = 0; }
static int wlen(const wchar *s) { int n = 0; while (s[n]) n++; return n; }

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
struct findprop { int prop, cls; int off; u32 obj; };
static int v_findprop(u32 o, int name, u32 cls, void *c) {
    struct findprop *f = c; (void)cls;
    u32 outer;
    if (name != f->prop) return 0;
    outer = rd32(o + 0x18);
    /* the outer must be the class itself, not a package of the same name: class PersistentStore lives in package
       KWGame, and KWGame is also a class, so "KWGame.PersistentStore" alone is ambiguous */
    if ((int)rd32(outer + 0x20) == f->cls && (int)rd32(rd32(outer + 0x24) + 0x20) == N_Class) { f->off = (int)rd32(o + 0x4C); f->obj = o; return 1; }
    return 0;
}
/* the UProperty object itself (0 if not found) */
static u32 prop_object(const wchar *cls, const wchar *prop) {
    struct findprop f; f.prop = name_index(prop); f.cls = name_index(cls); f.off = -1; f.obj = 0;
    if (f.prop >= 0 && f.cls >= 0) for_each_object(v_findprop, &f);
    return f.obj;
}
/* byte offset of property `prop` declared in class `cls` (-1 if not found) */
static int prop_offset(const wchar *cls, const wchar *prop) {
    struct findprop f; f.prop = name_index(prop); f.cls = name_index(cls); f.off = -1; f.obj = 0;
    if (f.prop >= 0 && f.cls >= 0) for_each_object(v_findprop, &f);
    return f.off;
}
/* member of a struct (the outer is a Struct, not a Class); struct names here are unique, so no class check */
static int v_findmember(u32 o, int name, u32 cls, void *c) {
    struct findprop *f = c; (void)cls;
    if (name == f->prop && (int)rd32(rd32(o + 0x18) + 0x20) == f->cls) { f->off = (int)rd32(o + 0x4C); f->obj = o; return 1; }
    return 0;
}
static int struct_member_offset(const wchar *strct, const wchar *prop) {
    struct findprop f; f.prop = name_index(prop); f.cls = name_index(strct); f.off = -1; f.obj = 0;
    if (f.prop >= 0 && f.cls >= 0) for_each_object(v_findmember, &f);
    return f.off;
}
static int is_a_cls(u32 c, u32 want) { int guard = 64; while (c && guard--) { if (c == want) return 1; c = rd32(c + 0x2C); } return 0; }

/* every Player (viewport) object: Player.Actor is the live PlayerController */
struct players { u32 clsPlayer, clsClass; u32 found[4]; int n; u32 known[512]; int nknown; };
static int v_players(u32 o, int name, u32 cls, void *c) {
    struct players *s = c; int k; (void)name;
    if (!cls || cls == s->clsClass || !s->clsPlayer) return 0;
    for (k = 0; k < s->nknown; k++) if (s->known[k] == cls) return 0;
    if (is_a_cls(cls, s->clsPlayer)) { if (s->n < 4) s->found[s->n++] = o; return 0; }
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
/* canonical spelling of a known map file name (case-insensitive), or 0 */
static const wchar *known_map(const wchar *name) {
    unsigned k;
    for (k = 0; k < sizeof MAP_IDS / sizeof MAP_IDS[0]; k++) if (wieq(name, MAP_IDS[k][1])) return MAP_IDS[k][1];
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
