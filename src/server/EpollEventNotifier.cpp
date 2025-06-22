#include "EpollEvent.hpp"
#include "EpollEventNotifier.hpp"
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>

EpollEventNotifier::EpollEventNotifier() {
	epoll_fd_ = epoll_create(MAX_FD_FOR_EPOLL);
}

EpollEventNotifier::~EpollEventNotifier() {
	if (epoll_fd_ >= 0) close(epoll_fd_);
}

types::Result<int, int> EpollEventNotifier::registerFd(int fd, void* userData) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.ptr = userData;
    int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1)
        return types::err(errno);
    return types::ok(ret);
}

types::Result<int, int> EpollEventNotifier::unregisterFd(int fd) {
	int ret = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, NULL);
	 if (ret == -1)
        return types::err(errno);
    return types::ok(ret);
}

types::Result<int, int> EpollEventNotifier::wait(EpollEvent *ev) {
	ev->events = EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR;
	int num_of_events = epoll_wait(epoll_fd_, ev ,MAX_FD_FOR_EPOLL, NON_BLOCKING_TIMEOUT);
	if (num_of_events < 0)
		return types::err(errno);
	return types::ok(num_of_events);
}

