#include "server/fileDescriptor/FdUtils.hpp"
#include <fcntl.h>
#include <unistd.h>
#include "utils/logger.hpp"

int FdUtils::set_nonblock_and_cloexec(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl == -1) {
        return -1;
    }
    if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) == -1) {
        return -1;
    }
    int fdfl = fcntl(fd, F_GETFD, 0);
    if (fdfl == -1) {
        return -1;
    }
    if (fcntl(fd, F_SETFD, fdfl | FD_CLOEXEC) == -1) {
        return -1;
    }
    return 0;
}

void FdUtils::safe_fd_close(int fd) {
    if (fd >= 0)
        ::close(fd);
    else 
        LOG_WARN("Fdutils::safe_fd_close: File descriptor is invalid or already closed");
}
