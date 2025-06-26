#pragma once

#include "IEventNotifier.hpp"
#include "../utils/types/result.hpp"
#include <sys/epoll.h>
#include <vector>
#include "ISocket.hpp"

#define MAX_FD_FOR_EPOLL 1024
#define NON_BLOCKING_TIMEOUT 0

class EpollEventNotifier : public IEventNotifier {
	private: 
		int epoll_fd_;

	public:
		EpollEventNotifier() {}
    	~EpollEventNotifier() {}

   	 	virtual types::Result<int, int> registerFd(int fd, ISocket socket);
    	virtual types::Result<int, int> unregisterFd(int fd);
    	virtual types::Result<int, int> wait(int timeoutMillis);
};
