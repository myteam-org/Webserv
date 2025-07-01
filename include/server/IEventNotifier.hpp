#pragma once

#include "utils/types/result.hpp"
#include "EpollEvent.hpp"
#include <vector>
#include "io/base/FileDescriptor.hpp"

class IEventNotifier {
public:
    virtual types::Result<int, int> registerFd(FileDescriptor &fd, EpollEvent &ev) = 0;
    virtual types::Result<int, int> unregisterFd(FileDescriptor &fd) = 0;
    virtual types::Result<int, int> modifyFd(FileDescriptor &fd, EpollEvent &ev) = 0;
    virtual types::Result<std::vector<EpollEvent>, int> wait() = 0;
};
