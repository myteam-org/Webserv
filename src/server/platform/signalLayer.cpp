#include "signalLayer.hpp"

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
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
    } else {
        errno = saved;
        return;
    }
    int fd = static_cast<int>(sWriteFd_);
    if (fd >= 0) {
        (void)write(fd, &code, 1);
    }
    errno = saved;
}

} // namespace platform
