#pragma once

#include "result.hpp"
#include "EpollEvent.hpp"
#include <vector>

class IEventNotifier {
public:
    virtual types::Result<int, int> registerFd(int fd, EpollEvent &ev) = 0;
    virtual types::Result<int, int> unregisterFd(int fd) = 0;
    virtual types::Result<std::vector<EpollEvent>, int> wait() = 0;
};
