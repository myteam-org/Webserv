#include "signalLayer.hpp"

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <errno.h>

#include "utils/string.hpp"

namespace platform {

volatile sig_atomic_t SignalLayer::sWriteFd_ = -1;
volatile sig_atomic_t SignalLayer::sFlags_ = 0;

SignalLayer::SignalLayer() : rfd_(-1), wfd_(-1) {}

SignalLayer::~SignalLayer() { shutdown(); }

namespace {
static bool set_cloexec_nonblock_(int fd, std::string* err) {
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

static bool setHandler_(int signo, void (*handler)(int), int flags, std::string* err) {
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    sa.sa_flags = flags;
    if (sigaction(signo, &sa, NULL) < 0) {
        if (err) *err = "sigaction(" + utils::toString(signo) + ") failed";
        return false;
    }
    return true;
}
} // namespace

bool SignalLayer::init(std::string* err) {
    shutdown();

    int pfd[2];
    if(pipe(pfd) != 0) {
        if (err) *err = "pipe failed";
        return false;
    }
    rfd_ = pfd[0];
    wfd_ = pfd[1];

    if (!set_cloexec_nonblock_(rfd_, err)) return shutdown(), false;
    if (!set_cloexec_nonblock_(wfd_, err)) return shutdown(), false;

    if (!setHandler_(SIGPIPE, SIG_IGN, 0, err)) {
        return shutdown(), false;
    }
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = &SignalLayer::onSignal;
    sa.sa_flags = SA_RESTART;    
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        if (err) return *err = "sigaction(SIGINT) failed", shutdown(), false;
    }
    if (sigaction(SIGTERM, &sa, NULL) < 0) {
        if (err) return *err = "sigaction(SIGTERM) failed", shutdown(), false;
    }
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        if (err) return *err = "sigaction(SIGHUP) failed", shutdown(), false;
    }
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        if (err) return *err = "sigaction(SIGCHLD) failed", shutdown(), false;
    }
    sWriteFd_ = static_cast<sig_atomic_t>(wfd_);
    sFlags_ = 0;
    return true;
}

// 読み取り側fd(epoll/kqueue に登録するのはこれ)
int SignalLayer::fd() const { return rfd_; }

// １イベント分だけ読み出して分類。読めなければfalse
bool SignalLayer::drainOnce(SignalAction* act) const {
    if (rfd_ < 0) {
        return false;
    }
    unsigned char code = 0;
    ssize_t byte = read(rfd_, &code, 1);
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

bool SignalLayer::takePending(SignalAction* act) {
    const sig_atomic_t flag = sFlags_;

    if (flag == 0) {
        return false;
    }
    if ((flag & 0x01) != 0) {
        sFlags_ &= ~0x01;
        if (act) {
            *act = kSigGracefulStop;
        }
        return true;
    }
    if ((flag & 0x02) != 0) {
        sFlags_ &= ~0x02;
        if (act) {
            *act = kSigReload;
        }
        return true;
    }
    if ((flag & 0x04) != 0) {
        sFlags_ &= ~0x04;
        if (act) {
            *act = kSigChild;
        }
        return true;
    }
    return false;
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

void SignalLayer::onSignal(int signo) {
    int saved = errno;
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
    }
    int fd = static_cast<int>(sWriteFd_);
    if (fd >= 0) {
        (void)write(fd, &code, 1);
    }
    errno = saved;
}

} // namespace platform
