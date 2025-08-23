// tests/signal_layer_test.cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/wait.h>

#include "signalLayer.hpp"

// 簡易アサート
static int g_fail = 0;
static void expect_true(bool cond, const char* msg) {
    if (!cond) {
        std::fprintf(stderr, "[FAIL] %s\n", msg);
        ++g_fail;
    } else {
        std::printf("[ OK ] %s\n", msg);
    }
}

static bool add_epoll_in(int ep, int fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    return epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev) == 0;
}

static bool wait_epoll_in(int ep, int watched_fd, int timeout_ms) {
    const int slice = 50;                       // 50ms刻みでリトライ
    int remaining = timeout_ms;

    while (remaining >= 0) {
        struct epoll_event evs[8];
        int t = (remaining < slice) ? remaining : slice;
        int n = epoll_wait(ep, evs, 8, t);

        if (n > 0) {
            for (int i = 0; i < n; ++i) {
                if (evs[i].data.fd == watched_fd && (evs[i].events & EPOLLIN)) {
                    return true;
                }
            }
            // 他FDのイベントだった → 残り時間で再試行
        } else if (n == 0) {
            // タイムアウト片スライス経過
        } else {
            // n < 0（EINTR想定）: errnoを見ずにリトライ
        }
        remaining -= slice;
    }
    return false;
}

static bool drain_all_actions(platform::SignalLayer* sig, platform::SignalAction* out) {
    // self-pipe から読み尽くし、なければ pending フラグを消費
    platform::SignalAction a;
    bool got = false;
    while (sig->drainOnce(&a)) { *out = a; got = true; }
    while (sig->takePending(&a)) { *out = a; got = true; }
    return got;
}

int main() {
    std::printf("== SignalLayer self-pipe / epoll test ==\n");

    platform::SignalLayer sig;
    std::string err;
    bool ok = sig.init(&err);
    expect_true(ok, ok ? "SignalLayer::init()" : err.c_str());
    if (!ok) return 1;

    int ep = epoll_create(16);
    expect_true(ep >= 0, "epoll_create");
    if (ep < 0) return 1;

    expect_true(add_epoll_in(ep, sig.fd()), "epoll_ctl ADD SignalLayer fd");

    // --- 1) SIGINT → kSigGracefulStop
    raise(SIGINT);
    expect_true(wait_epoll_in(ep, sig.fd(), 200), "epoll_wait after SIGINT");
    platform::SignalAction act = platform::kSigNone;
    expect_true(drain_all_actions(&sig, &act), "drain SIGINT action");
    expect_true(act == platform::kSigGracefulStop, "action == kSigGracefulStop");

    // --- 2) SIGHUP → kSigReload
    raise(SIGHUP);
    expect_true(wait_epoll_in(ep, sig.fd(), 200), "epoll_wait after SIGHUP");
    act = platform::kSigNone;
    expect_true(drain_all_actions(&sig, &act), "drain SIGHUP action");
    expect_true(act == platform::kSigReload, "action == kSigReload");

    // --- 3) SIGCHLD（fork→子即終了）→ kSigChild
    pid_t c = fork();
    if (c == 0) { _exit(0); } // 子
    expect_true(c > 0, "fork()");
    // 子終了に伴う SIGCHLD を待つ
    expect_true(wait_epoll_in(ep, sig.fd(), 500), "epoll_wait after child exit (SIGCHLD)");
    act = platform::kSigNone;
    expect_true(drain_all_actions(&sig, &act), "drain SIGCHLD action");
    expect_true(act == platform::kSigChild, "action == kSigChild");
    // ゾンビ掃除（errnoは見ない方針でもOK：戻り値のみで十分）
    int st = 0;
    (void)waitpid(c, &st, 0);

    // --- 4) 何もしない時はイベントが来ない
    expect_true(!wait_epoll_in(ep, sig.fd(), 50), "no spurious EPOLLIN");

    // --- 5) shutdown の確認
    sig.shutdown();
    expect_true(sig.fd() == -1, "fd() == -1 after shutdown");

    std::printf("\nResult: %s (%d failure(s))\n",
                (g_fail == 0 ? "ALL PASSED" : "FAILED"), g_fail);
    return g_fail == 0 ? 0 : 1;
}
