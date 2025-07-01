#pragma once

#include "IEventNotifier.hpp"
#include "../utils/types/result.hpp"
#include <sys/epoll.h>
#include <vector>
#include "io/base/FileDescriptor.hpp"

class EpollEventNotifier : public IEventNotifier {
private: 
    FileDescriptor epoll_fd_;
    static const int kEpollError = -1;
    static const int kMaxAssociatedFD = 1024;
    static const int kTimeoutImmediate  = 0;
public:
    EpollEventNotifier();
    ~EpollEventNotifier();
    virtual types::Result<int, int> registerFd(FileDescriptor &fd, EpollEvent &ev);
    virtual types::Result<int, int> unregisterFd(FileDescriptor &fd);
    virtual types::Result<std::vector<EpollEvent>, int> wait();
    virtual types::Result<int, int> modifyFd(FileDescriptor &fd, EpollEvent &ev);
};
