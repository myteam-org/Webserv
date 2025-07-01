#include "io/base/FileDescriptor.hpp"

FileDescriptor::FileDescriptor(int fd) : fd_(fd) {}

FileDescriptor::~FileDescriptor() {
    if (fd_ > kInvalidFD) {
        ::close(fd_);
    }
}

types::Option<int> FileDescriptor::getFd() const {
    if (fd_ == kInvalidFD) {
        return types::Option<int>(types::none);
    }
    return types::Option<int>(types::some(fd_));
}

void FileDescriptor::setFd(int fd) {
    if (fd_ > kInvalidFD) {
        ::close(fd_);
    }
    fd_ = fd;
}
