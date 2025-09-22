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
    static const int kEpollTimeoutMS  = 200;
    std::vector<struct epoll_event> events_;
public:
    EpollEventNotifier();
    ~EpollEventNotifier();
    types::Result<types::Unit, int> open();
    virtual types::Result<int, int> registerFd(FileDescriptor &fd, EpollEvent &ev);
    virtual types::Result<int, int> unregisterFd(FileDescriptor &fd);
    virtual types::Result<std::vector<EpollEvent>, int> wait();
    virtual types::Result<int, int> modifyFd(FileDescriptor &fd, EpollEvent &ev);
    types::Result<int,int> add(FileDescriptor& fd, uint32_t mask);
    types::Result<int,int> mod(FileDescriptor& fd, uint32_t mask);
    types::Result<int,int> add(int fd, uint32_t mask);
    types::Result<int,int> mod(int fd, uint32_t mask);
    types::Result<int,int> del(int fd);


};
