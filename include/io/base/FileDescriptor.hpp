#pragma once

#include <cstddef>
#include "option.hpp"
#include <unistd.h>

class FileDescriptor {
    public:
        static const int kInvalidFD = -1;
        explicit FileDescriptor(int fd = kInvalidFD);
        ~FileDescriptor();
        types::Option<int> getFd() const;
        void setFd(int fd);
    private:
        FileDescriptor(const FileDescriptor&);
        int fd_;
};
