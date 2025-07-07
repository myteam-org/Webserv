#pragma once

#include "utils/types/result.hpp"
#include "io/base/FileDescriptor.hpp"

class ISocket {
public:
    virtual int getRawFd() const = 0;
};
