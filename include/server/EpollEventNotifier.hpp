#pragma once

#include "IEventNotifier.hpp"
#include "../utils/types/result.hpp"
#include <sys/epoll.h>
#include <vector>

class EpollEventNotifier : public IEventNotifier {
	private: 
		int epoll_fd_;
		static const int kEpollError = -1;
		static const int kMaxAssociatedFD = 1024;
		static const int kNonBlockingTimeout = 0;

	public:
		EpollEventNotifier() {}
    	~EpollEventNotifier() {}

   	 	virtual types::Result<int, int> registerFd(int fd, EpollEvent &ev);
    	virtual types::Result<int, int> unregisterFd(int fd);
    	virtual types::Result<std::vector<EpollEvent>, int> wait();
};
