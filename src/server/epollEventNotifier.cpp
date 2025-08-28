#include "EpollEvent.hpp"
#include "EpollEventNotifier.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#include <vector>
#include "try.hpp"
#include <cerrno> 
#include <fcntl.h>
EpollEventNotifier::EpollEventNotifier()
    : epoll_fd_(-1) {}

types::Result<types::Unit, int> EpollEventNotifier::open() {
    int ep = ::epoll_create(kMaxAssociatedFD);
    if (ep < 0) return ERR(errno);
    int flags = ::fcntl(ep, F_GETFD);
    if (flags >= 0) ::fcntl(ep, F_SETFD, flags | FD_CLOEXEC);
    epoll_fd_.setFd(ep);
    if (events_.empty()) events_.resize(kMaxAssociatedFD);
    return types::ok();
}

EpollEventNotifier::~EpollEventNotifier() {
}

types::Result<int, int> EpollEventNotifier::registerFd(FileDescriptor &fd, EpollEvent &ev) {
    const types::Option<int> epfdOpt = epoll_fd_.getFd();
    if (!epfdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    const types::Option<int> targetFdOpt = fd.getFd();
    if (!targetFdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    const int ret = epoll_ctl(epfdOpt.unwrap(), EPOLL_CTL_ADD, targetFdOpt.unwrap(), ev.raw());
    if (ret == kEpollError) {
        return ERR(errno);
    }
    return OK(ret);
}

types::Result<int, int> EpollEventNotifier::unregisterFd(FileDescriptor &fd) {
    const types::Option<int> epfdOpt = epoll_fd_.getFd();
    if (!epfdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    const types::Option<int> targetFdOpt = fd.getFd();
    if (!targetFdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    const int ret = epoll_ctl(epfdOpt.unwrap(), EPOLL_CTL_DEL, targetFdOpt.unwrap(), NULL);
    if (ret == kEpollError) {
        return ERR(errno);
    }
    return OK(ret);
}

types::Result<int, int> EpollEventNotifier::modifyFd(FileDescriptor &fd, EpollEvent &ev) {
    const types::Option<int> epfdOpt = epoll_fd_.getFd();
    if (!epfdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    const types::Option<int> targetFdOpt = fd.getFd();
    if (!targetFdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    const int ret = epoll_ctl(epfdOpt.unwrap(), EPOLL_CTL_MOD, targetFdOpt.unwrap(), ev.raw());
    if (ret == kEpollError) {
        return ERR(errno);
    }
    return OK(ret);
}

types::Result<std::vector<EpollEvent>, int> EpollEventNotifier::wait() {
    const types::Option<int> epfdOpt = epoll_fd_.getFd();
    if (!epfdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    if (events_.empty()) {
        events_.resize(kMaxAssociatedFD);
    }
    int n;
    for (;;) {
        n = ::epoll_wait(epfdOpt.unwrap(), &events_[0], (int)events_.size(), kEpollTimeoutMS);
        if (n >= 0) 
            break;
        if (errno == EINTR)
            continue;
        return ERR(errno);
    }
    std::vector<EpollEvent> out;
    out.reserve(n);
    for (int i = 0; i < n; ++i) {
        out.push_back(EpollEvent(events_[i]));
    }
    return OK(out);
}

types::Result<int,int> EpollEventNotifier::add(FileDescriptor &fd, uint32_t mask) {
    const types::Option<int> targetFdOpt = fd.getFd();
    if (!targetFdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    EpollEvent ev(mask, /*userData*/ 0);
    ev.setUserFd(targetFdOpt.unwrap());
    return registerFd(fd, ev);
}

types::Result<int,int> EpollEventNotifier::mod(FileDescriptor &fd, uint32_t mask) {
    const types::Option<int> targetFdOpt = fd.getFd();
    if (!targetFdOpt.canUnwrap()) {
        return ERR(EINVAL);
    }
    EpollEvent ev(mask, /*userData*/ 0);
    ev.setUserFd(targetFdOpt.unwrap());
    return modifyFd(fd, ev);
}


