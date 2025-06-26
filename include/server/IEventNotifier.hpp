#pragma once

#include "result.hpp"
#include "EpollEvent.hpp"

class IEventNotifier {
public:
    virtual types::Result<int, int> registerFd(int fd, unsigned int events, void* userData) = 0;
    virtual types::Result<int, int> unregisterFd(int fd) = 0;
    virtual types::Result<int, int> wait(EpollEvent ev) = 0;
};
