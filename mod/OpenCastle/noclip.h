/*
 * Noclip (TEST ONLY, like the debug panel: remove from release builds). F6, or the debug panel's "Noclip" row, toggles
 * it. While on, the player flies through walls, floors and actors:
 *   move      the game's own movement keys (camera-relative, read from the controller's aForward / aStrafe; WASD or
 *             the arrow keys are read directly as a fallback)
 *   up/down   X, Space, E or Page Up / C, Left Ctrl, Q or Page Down
 *   faster    hold Shift (x4)
 * Turning it off drops the player from where they are (falling physics, collision back on), so turn it off above a
 * floor. Nothing is picked up and no trigger fires while it's on (collision is off): land next to the item.
 *
 * How: on the game thread, right after the player controller's PlayerTick (once a frame). The pawn gets collision off
 * (AActor::SetCollision), bCollideWorld cleared and PHYS_None (AActor::setPhysics), so neither the engine's physics nor
 * the controller's walking code moves it; then it is placed with ULevel::FarMoveActor (no encroachment check), which
 * also keeps its zone up to date. Velocity / Acceleration are zeroed every frame in case the game sets them.
 */
typedef void (__thiscall *setphys_fn)(void *actor, u8 phys, void *floor, float nx, float ny, float nz);   /* FVector by value */
typedef int (__thiscall *farmove_fn)(void *level, void *actor, float x, float y, float z, int test, int noCheck, int attachMove);
static setphys_fn gSetPhysics; static farmove_fn gFarMove;
static volatile int gNoclip;            /* wanted (key thread / panel) */
static int gNoclipOn; static u32 gNcPawn;  /* applied (game thread) */
static int gNcCol[3] = { 1, 1, 1 };   /* the pawn's bCollideActors / bBlockActors / bBlockPlayers before noclip */
static int offNcBool[3] = { -1, -1, -1 }; static u32 maskNcBool[3];
static int offNcPhys = -1, offNcVel = -1, offNcAcc = -1, offNcRot = -1, offNcCam = -1, offNcFwd = -1, offNcStrafe = -1,
           offNcCW = -1, gNcReady; static u32 maskNcCW;

/* worker, every 2 s until ready */
static void noclip_lookups(void) {
    HANDLE eng; static wchar b[260]; u32 bp;
    if (gNcReady) return;
    engine_exports();
    if (offNcPhys < 0) offNcPhys = prop_offset(L"Actor", L"Physics");
    if (offNcVel < 0) offNcVel = prop_offset(L"Actor", L"Velocity");
    if (offNcAcc < 0) offNcAcc = prop_offset(L"Actor", L"Acceleration");
    if (offNcRot < 0) offNcRot = prop_offset(L"Actor", L"Rotation");
    if (offNcCam < 0) offNcCam = prop_offset(L"KWHeroController", L"Camera");
    if (offNcFwd < 0) offNcFwd = prop_offset(L"PlayerController", L"aForward");
    if (offNcStrafe < 0) offNcStrafe = prop_offset(L"PlayerController", L"aStrafe");
    if (offNcCW < 0 && (bp = prop_object(L"Actor", L"bCollideWorld")) != 0) {   /* UBoolProperty.BitMask at +0x70 */
        u32 m = rd32(bp + 0x70);
        if (m && !(m & (m - 1))) { offNcCW = (int)rd32(bp + 0x4C); maskNcCW = m; }
    }
    {   static const wchar *const B[3] = { L"bCollideActors", L"bBlockActors", L"bBlockPlayers" }; int i;
        for (i = 0; i < 3; i++) if (offNcBool[i] < 0 && (bp = prop_object(L"Actor", B[i])) != 0) {
            u32 m = rd32(bp + 0x70);
            if (m && !(m & (m - 1))) { offNcBool[i] = (int)rd32(bp + 0x4C); maskNcBool[i] = m; }
        }
    }
    eng = GetModuleHandleW(L"Engine.dll");
    if (eng) {
        gSetPhysics = (setphys_fn)GetProcAddress(eng, "?setPhysics@AActor@@UAEXEPAV1@VFVector@@@Z");
        gFarMove = (farmove_fn)GetProcAddress(eng, "?FarMoveActor@ULevel@@UAEHPAVAActor@@VFVector@@HHH@Z");
    }
    if (offNcPhys <= 0 || offNcVel <= 0 || offNcAcc <= 0 || offNcRot <= 0 || offNcCW <= 0 || offXLevel <= 0 || !gSetCollision
        || !gSetPhysics || !gFarMove) return;
    gNcReady = 1;
    wsprintfW(b, L"noclip: ready (physics=%d velocity=%d accel=%d rotation=%d camera=%d aForward=%d aStrafe=%d collideworld=%d/%x); F6 toggles",
              offNcPhys, offNcVel, offNcAcc, offNcRot, offNcCam, offNcFwd, offNcStrafe, offNcCW, maskNcCW);
    log_line(b);
}

/* x87 only (no CRT): sin / cos of an angle in radians, |a| <= pi after reduction */
static float nc_sin(float a) {
    const float PI = 3.14159265f;
    while (a > PI) a -= 2 * PI;
    while (a < -PI) a += 2 * PI;
    if (a > PI / 2) a = PI - a; else if (a < -PI / 2) a = -PI - a;
    { float a2 = a * a; return a * (1 - a2 / 6 * (1 - a2 / 20 * (1 - a2 / 42 * (1 - a2 / 72)))); }
}
static float nc_cos(float a) { return nc_sin(a + 1.57079633f); }
static int nc_key(int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; }

/* game thread, after the player controller's PlayerTick */
static void noclip_tick(u32 pc, float dt) {
    u32 pawn, xlevel, cam; float loc[3], f = 0, s = 0, up = 0, yaw, cy, sy, mx, my, mz, len, speed; int rot[3], k; u32 pid = 0;
    static int inputLogged; static wchar b[200];
    if (!gNcReady || !pc) return;
    pawn = P(pc + offPawn);
    if (gNoclipOn && (!gNoclip || pawn != gNcPawn)) {   /* off (or the pawn changed: level load) */
        if (pawn && pawn == gNcPawn) {
            gSetCollision((void *)pawn, gNcCol[0], gNcCol[1], gNcCol[2]);
            *(u32 *)(pawn + offNcCW) |= maskNcCW;
            gSetPhysics((void *)pawn, 2 /*PHYS_Falling*/, 0, 0, 0, 0);
            log_line(L"noclip: off");
        }
        gNoclipOn = 0; gNcPawn = 0;
    }
    if (!gNoclip || !pawn) return;
    if (!gNoclipOn) {
        for (k = 0; k < 3; k++) gNcCol[k] = offNcBool[k] > 0 ? (*(u32 *)(pawn + offNcBool[k]) & maskNcBool[k]) != 0 : 1;
        gSetCollision((void *)pawn, 0, 0, 0);
        *(u32 *)(pawn + offNcCW) &= ~maskNcCW;
        gSetPhysics((void *)pawn, 0 /*PHYS_None*/, 0, 0, 0, 0);
        gNoclipOn = 1; gNcPawn = pawn;
        log_line(L"noclip: on (move keys, X / Space / E / PgUp up, C / LCtrl / Q / PgDn down, Shift faster; F6 off)");
    }
    if (*(u8 *)(pawn + offNcPhys) != 0) gSetPhysics((void *)pawn, 0, 0, 0, 0, 0);   /* the game switched it back */
    ((float *)(pawn + offNcVel))[0] = ((float *)(pawn + offNcVel))[1] = ((float *)(pawn + offNcVel))[2] = 0;
    ((float *)(pawn + offNcAcc))[0] = ((float *)(pawn + offNcAcc))[1] = ((float *)(pawn + offNcAcc))[2] = 0;
    if (dt <= 0 || dt > 0.25f) return;
    GetWindowThreadProcessId(GetForegroundWindow(), &pid);
    if (pid != GetCurrentProcessId()) return;
    /* direction: the controller's input axes (the game's own bindings), else the keys themselves */
    if (offNcFwd > 0 && offNcStrafe > 0) { f = *(float *)(pc + offNcFwd); s = *(float *)(pc + offNcStrafe); }
    if ((f != 0 || s != 0) && inputLogged < 3) { inputLogged++; wsprintfW(b, L"noclip: input axes forward %d strafe %d", (int)f, (int)s); log_line(b); }
    if (f == 0 && s == 0) {
        f = (float)((nc_key('W') || nc_key(0x26)) - (nc_key('S') || nc_key(0x28)));
        s = (float)((nc_key('D') || nc_key(0x27)) - (nc_key('A') || nc_key(0x25)));
    }
    up = (float)((nc_key('X') || nc_key(0x20) || nc_key('E') || nc_key(0x21)) - (nc_key('C') || nc_key(0xA2 /*LCtrl*/) || nc_key('Q') || nc_key(0x22)));
    /* camera-relative: the third-person camera's yaw, else the pawn's */
    cam = offNcCam > 0 ? P(pc + offNcCam) : 0;
    rot[1] = *(int *)((cam ? cam : pawn) + offNcRot + 4);
    yaw = (float)(rot[1] & 0xFFFF) * (6.28318531f / 65536.0f);
    cy = nc_cos(yaw); sy = nc_sin(yaw);
    mx = f * cy - s * sy; my = f * sy + s * cy;   /* X = (cos, sin), Y (right) = (-sin, cos) */
    len = mx * mx + my * my;
    if (len > 0) {   /* normalise: sqrt by Newton's method */
        float r = len > 1 ? len : 1;
        for (k = 0; k < 8; k++) r = 0.5f * (r + len / r);
        mx /= r; my /= r;
    }
    mz = up;
    if (mx == 0 && my == 0 && mz == 0) return;
    speed = (nc_key(0x10) ? 280000.0f : 70000.0f) * dt;   /* units per second; Shift x4 */
    xlevel = P(pawn + offXLevel);
    if (!xlevel) return;
    loc[0] = ((float *)(pawn + offLoc))[0] + mx * speed;
    loc[1] = ((float *)(pawn + offLoc))[1] + my * speed;
    loc[2] = ((float *)(pawn + offLoc))[2] + mz * speed * 0.1f;   /* vertical: 10% of the horizontal speed */
    gFarMove((void *)xlevel, (void *)pawn, loc[0], loc[1], loc[2], 0, 1, 0);
}
