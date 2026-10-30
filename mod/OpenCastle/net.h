/*
 * Local connection to the Archipelago client (apworld/hp3pc/client.py), the HP2PC_AP way: the client listens on
 * 127.0.0.1:AP_PORT, the mod connects to it and keeps reconnecting while it isn't there. Newline-terminated ASCII
 * lines both ways (protocol: ap.h).
 *
 * Its own thread (net_thread): connect, then select() on the socket with a short timeout, so it both reads and
 * flushes what the other threads queued. Lines in go to a ring the worker drains (ap_poll); lines out are queued
 * from any thread (net_send). Nothing here touches the game.
 */
#define AP_PORT 42780
#define NET_INMAX 256       /* inbound lines held for the worker */
#define NET_LINE 1024       /* longest line, either way */
#define NET_OUTMAX 65536    /* outbound bytes queued */

typedef unsigned int SOCKET_T;
struct sockaddr_in_t { short family; unsigned short port; u32 addr; char zero[8]; };
struct timeval_t { long sec, usec; };
struct fdset_t { u32 count; SOCKET_T fd[1]; };
IMP int WINAPI WSAStartup(unsigned short, void *);
IMP SOCKET_T WINAPI socket(int, int, int);
IMP int WINAPI connect(SOCKET_T, const void *, int);
IMP int WINAPI send(SOCKET_T, const char *, int, int);
IMP int WINAPI recv(SOCKET_T, char *, int, int);
IMP int WINAPI closesocket(SOCKET_T);
IMP int WINAPI select(int, struct fdset_t *, struct fdset_t *, struct fdset_t *, const struct timeval_t *);
IMP unsigned short WINAPI htons(unsigned short);

static volatile int gNetUp;                       /* connected to the client */
static volatile int gNetLock;                     /* spinlock for the outbound buffer */
static char gNetOut[NET_OUTMAX]; static volatile int gNetOutN;
static char gNetIn[NET_INMAX][NET_LINE]; static volatile int gNetInHead, gNetInTail;   /* ring: net thread writes head */
static volatile int gNetGen;                      /* +1 on every new connection (ap.h says HELLO again) */

static void net_lock(void) { while (__sync_lock_test_and_set(&gNetLock, 1)) Sleep(0); }
static void net_unlock(void) { __sync_lock_release(&gNetLock); }
/* any thread: queue one line (no newline in `s`); dropped if the client isn't connected or the buffer is full */
static void net_send(const char *s) {
    int n = 0;
    while (s[n] && n < NET_LINE - 2) n++;
    if (!gNetUp) return;
    net_lock();
    if (gNetOutN + n + 1 <= NET_OUTMAX) { memcpy(gNetOut + gNetOutN, s, (unsigned)n); gNetOut[gNetOutN + n] = '\n'; gNetOutN += n + 1; }
    net_unlock();
}
/* worker: next inbound line, or 0 */
static int net_recv_line(char *out) {
    int t = gNetInTail, i;
    if (t == gNetInHead) return 0;
    for (i = 0; i < NET_LINE - 1 && gNetIn[t][i]; i++) out[i] = gNetIn[t][i];
    out[i] = 0;
    gNetInTail = (t + 1) % NET_INMAX;
    return 1;
}

static u32 WINAPI net_thread(void *arg) {
    static char wsa[512], buf[4096], line[NET_LINE], out[NET_OUTMAX];
    struct sockaddr_in_t sa; SOCKET_T s; int ln = 0, logged = 0; static wchar b[160];
    (void)arg;
    if (WSAStartup(0x0202, wsa) != 0) { log_line(L"ap: WSAStartup failed; no connection to the client"); return 0; }
    for (;;) {
        s = socket(2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 6 /*TCP*/);
        if (s == (SOCKET_T)~0u) { Sleep(3000); continue; }
        memset(&sa, 0, sizeof sa);
        sa.family = 2; sa.port = htons(AP_PORT); sa.addr = 0x0100007F;   /* 127.0.0.1 */
        if (connect(s, &sa, sizeof sa) != 0) {
            closesocket(s);
            if (!logged) { wsprintfW(b, L"ap: no client on 127.0.0.1:%d yet; retrying every 3 s", AP_PORT); log_line(b); logged = 1; }
            Sleep(3000); continue;
        }
        wsprintfW(b, L"ap: connected to the client (127.0.0.1:%d)", AP_PORT); log_line(b);
        logged = 0; ln = 0;
        net_lock(); gNetOutN = 0; net_unlock();
        gNetUp = 1; gNetGen++;
        for (;;) {
            struct fdset_t rd; struct timeval_t tv = { 0, 50000 }; int r, n, i;
            /* flush */
            net_lock(); n = gNetOutN; if (n) { memcpy(out, gNetOut, (unsigned)n); gNetOutN = 0; } net_unlock();
            for (i = 0; i < n; ) { int k = send(s, out + i, n - i, 0); if (k <= 0) { n = -1; break; } i += k; }
            if (n < 0) break;
            rd.count = 1; rd.fd[0] = s;
            r = select(0, &rd, 0, 0, &tv);
            if (r < 0) break;
            if (r == 0) continue;
            n = recv(s, buf, sizeof buf, 0);
            if (n <= 0) break;
            for (i = 0; i < n; i++) {
                char c = buf[i];
                if (c == '\r') continue;
                if (c != '\n') { if (ln < NET_LINE - 1) line[ln++] = c; continue; }
                line[ln] = 0;
                if (ln) {   /* ring full: wait for the worker rather than drop a line */
                    int h = gNetInHead;
                    while ((h + 1) % NET_INMAX == gNetInTail) Sleep(20);
                    memcpy(gNetIn[h], line, (unsigned)ln + 1); gNetInHead = (h + 1) % NET_INMAX;
                }
                ln = 0;
            }
        }
        gNetUp = 0;
        closesocket(s);
        log_line(L"ap: lost the client; reconnecting");
        Sleep(1000);
    }
}
