#include "EpollEvent.hpp"
#include "EpollEventNotifier.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#include <vector>
#include "try.hpp"
#include <cerrno> 
EpollEventNotifier::EpollEventNotifier()
    : epoll_fd_(epoll_create(kMaxAssociatedFD)) {
    if (!epoll_fd_.getFd().canUnwrap()) {
        throw std::runtime_error("epoll_create failed");
    }
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
    std::vector<struct epoll_event> events(kMaxAssociatedFD);
    const int num_of_events = epoll_wait(epfdOpt.unwrap(), events.data(), kMaxAssociatedFD, kTimeoutImmediate);
    if (num_of_events <= kEpollError) {
        return ERR(errno);
    }
    std::vector<EpollEvent> ep_events;
    ep_events.reserve(num_of_events);
    for (int i = 0; i < num_of_events; i++) {
        ep_events.push_back(EpollEvent(events[i]));
    }
    return OK(ep_events);
}
