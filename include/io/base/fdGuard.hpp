#pragma once

#include <unistd.h>

namespace io {
namespace base {

class FdGuard {
   public:
    explicit FdGuard(int fd) : fd_(fd) {}
    ~FdGuard() {
        if (fd_ >= 0) {
            close(fd_);
        }
    }
    int get() const { return fd_; }

   private:
    FdGuard(const FdGuard&);
    FdGuard& operator=(const FdGuard&);

    int fd_;
};

} // namespace base
}// namespace io   