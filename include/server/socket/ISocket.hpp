#pragma once

#include "utils/types/result.hpp"
#include "io/base/FileDescriptor.hpp"

class ISocket {
public:
    virtual ~ISocket() = 0;
    virtual int getRawFd() const = 0;
};
