#include "EpollEvent.hpp"
#include "EpollEventNotifier.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#include <vector>

EpollEventNotifier::EpollEventNotifier() {
	epoll_fd_ = epoll_create(kMaxAssociatedFD);
}

EpollEventNotifier::~EpollEventNotifier() {
	if (epoll_fd_ >= 0) {
		close(epoll_fd_);
	}
}

types::Result<int, int> EpollEventNotifier::registerFd(int fd, EpollEvent &ev) {
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, ev.raw());
	if (ret == kEpollError) {
		return ERR(errno);
	}
    return OK(ret);
}

types::Result<int, int> EpollEventNotifier::unregisterFd(int fd) {
	int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
	if (ret == kEpollError) {
		return ERR(errno);
	}
    return OK(ret);
}

types::Result<std::vector<EpollEvent>, int> EpollEventNotifier::wait() {
	std::vector<struct epoll_event> events(kMaxAssociatedFD);
	int num_of_events = epoll_wait(epoll_fd_, events.data(), kMaxAssociatedFD, kNonBlockingTimeout);
	if (num_of_events <= kEpollError) {
		return ERR(errno);
	}
	std::vector<EpollEvent> ep_events;
	ep_events.reserve(num_of_events);
	for (int i = 0; i < num_of_events; i++) {
		EpollEvent e;
		*e.raw() = events[i];
		ep_events.push_back(e);
	}
	return OK(ep_events);
}
