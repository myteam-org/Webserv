#include "signalLayer.hpp"

#include <errno.h>

namespace platform {

volatile sig_atomic_t SignalLayer::sWriteFd_ = -1;
volatile sig_atomic_t SignalLayer::sFlags_ = 0;

SignalLayer::SignalLayer() : rfd_(-1), wfd_(-1) {}

SignalLayer::~SignalLayer() { shutdown(); }

namespace {
static bool set_cloexec_nonblock(int fd, std::string* err) {
    int fcl = fcntl(fd, F_GETFD, 0);
    
    if (fcl < 0) {
        if (err) *err = "fcntl(F_GETFD) failed";
        return false;
    }
    if (fcntl(fd, F_SETFD, fcl | FD_CLOEXEC) != 0) {
        if (err) *err = "fcntl(F_SETFD) failed";
        return false;
    }
    fcl = fcntl(fd, F_GETFL, 0);
    if (fcl < 0) {
        if (err) *err = "fcntl(F_GETFL) failed";
        return false;
    }
    if (fcntl(fd, F_SETFL, fcl | O_NONBLOCK) != 0) {
        if (err) *err = "fcntl(F_SETFL) failed";
        return false;
    }
    return true;
}
} // namespace

// 46~52 パイプ生成 → 読端/書端の保持
// 53~58 非ブロッキング化 & CLOEXEC（自己パイプ運用の定石）
// 59    ハンドラが書き込む先を静的に共有
// 61~63 プロセス全体で SIGPIPE を無視（SIG_IGN）に設定
// 64~75 シグナルハンドラの登録
bool SignalLayer::init(std::string* err) {
    shutdown();
    int pfd[2];
    if(pipe(pfd) != 0) {
        if (err) *err = "pipe failed";
        return false;
    }
    rfd_ = pfd[0];
    wfd_ = pfd[1];
    if (!set_cloexec_nonblock(rfd_, err)) {
        return failInitKeepErr(err);
    }
    if (!set_cloexec_nonblock(wfd_, err)) {
        return failInitKeepErr(err);
    }
    sWriteFd_ = static_cast<sig_atomic_t>(wfd_);
    sFlags_ = 0;
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) { // ignore SIGPIPE
        return failInit(err, "signal(SIGPIPE) failed");
    } 
    if (signal(SIGINT, &SignalLayer::onSignal) == SIG_ERR) {
        return failInit(err, "signal(SIGINT) failed");
    }
    if (signal(SIGTERM, &SignalLayer::onSignal) == SIG_ERR) {
        return failInit(err, "signal(SIGTERM) failed");
    }
    if (signal(SIGHUP, &SignalLayer::onSignal) == SIG_ERR) {
        return failInit(err, "signal(SIGHUP) failed");
    }
    if (signal(SIGCHLD, &SignalLayer::onSignal) == SIG_ERR) {
        return failInit(err, "signal(SIGCHLD) failed");
    }
    return true;
}

bool SignalLayer::failInitKeepErr(std::string* /*err*/) {
    shutdown();
    return false;
}

bool SignalLayer::failInit(std::string* err, const char* msg) {
    if (err) {
        *err = msg;
    }
    shutdown();
    return false;
}   

// epoll 監視用に読端を公開（= 呼び出し側が EPOLLIN 登録する意図）
int SignalLayer::fd() const { return rfd_; }

// １イベント分だけ読み出して分類。読めなければfalse
bool SignalLayer::drainOnce(SignalAction* act) const {
    if (rfd_ < 0) {
        return false;
    }
    unsigned char code = 0;
    const ssize_t byte = read(rfd_, &code, 1);
    if (byte <= 0) {
        return false;
    }
    if (act) {
        switch (code) {
            case 1:
                *act = kSigGracefulStop;
                break;
            case 2:
                *act = kSigReload;
                break;
            case 3:
                *act = kSigChild;
                break;
            default:
                *act = kSigNone;
                break;
        }
    }
    return true;
}

// パイプ溢れ・嵐の保険。sig_atomic_t のビットを見て 
// 0x01(stop)→0x02(reload)→0x04(child) の優先で1件だけ消費。
bool SignalLayer::takePending(SignalAction* act) {
    const sig_atomic_t flag = sFlags_;
    const int mask = static_cast<int>(flag & 0x07);

    if (mask == 0) {
        return false;
    }
    switch(mask) {
        case 0x01: case 0x03: case 0x05: case 0x07:
            sFlags_ &= ~0x01;
            if (act) {
                *act = kSigGracefulStop;
            }
            return true;
        case 0x02: case 0x06:
            sFlags_ &= ~0x02;
            if (act) { 
                *act = kSigReload;
            }
            return true;
        case 0x04:
            sFlags_ &= ~0x04;
            if (act) {
                *act = kSigChild;
            }
            return true;
        default:
            return false;
    }
}

void SignalLayer::shutdown() {
    sWriteFd_ = -1;
    sFlags_ = 0;
    if (wfd_ >= 0) {
        close(wfd_);
        wfd_ = -1;
    }
    if (rfd_ >= 0) {
        close(rfd_);
        rfd_ = -1;
    }
}

// ハンドラ内で“1バイト write”（＝self-pipe の書端へ通知）
// ハンドラで使っているのは write() のみ（async-signal-safe 関数 = 非同期安全）
// errno は保存→復元のみで分岐に使っていない
void SignalLayer::onSignal(int signo) {
    const int saved = errno;
    unsigned char code = 0;

    if (signo == SIGINT || signo == SIGTERM) {
        code = 1;
        sFlags_ |= 0x01;
    } else if (signo == SIGHUP) {
        code = 2;
        sFlags_ |= 0x02;
    } else if (signo == SIGCHLD) {
        code = 3;
        sFlags_ |= 0x04;
    } else {
        errno = saved;
        return;
    }
    const int fd = static_cast<int>(sWriteFd_);
    if (fd >= 0) {
        (void)write(fd, &code, 1);
    }
    errno = saved;
}

} // namespace platform

// 全体俯瞰
// self-pipe 方式：pipe() で作った書端(wfd_)をシグナルハンドラでwrite(1byte)、
//                読端(rfd_)を epoll で監視。
//非同期安全：      ハンドラは write() だけ（errno は保存→復元のみ、分岐に使わない）。
//drainOnce()：   EPOLLIN 到着時に1バイト読み出し→SignalAction に分類。
//                read()<=0 は「今回は読めない」として即 return（errno 不参照）。
//takePending()： パイプ溢れ・嵐の保険。sig_atomic_t のビットを見て 
//               0x01(stop)→0x02(reload)→0x04(child) の優先で1件だけ消費。
//shutdown()：FD を閉じ、静的フラグをリセット。