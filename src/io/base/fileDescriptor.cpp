#include "io/base/FileDescriptor.hpp"

FileDescriptor::FileDescriptor(int fd) : fd_(fd) {}

FileDescriptor::~FileDescriptor() {
    if (fd_ > kInvalidFD) {
        ::close(fd_);
    }
}

types::Option<int> FileDescriptor::getFd() const {
    if (fd_ == kInvalidFD) {
        return types::none<int>();
    }
    return types::some<int>(fd_);
}

void FileDescriptor::setFd(int fd) {
    if (fd_ > kInvalidFD) {
        ::close(fd_);
    }
    fd_ = fd;
}
