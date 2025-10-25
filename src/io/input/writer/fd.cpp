#include "io/input/writer/fd.hpp"
#include "io/input/writer/writer.hpp"
#include "utils/types/result.hpp"
#include <unistd.h>
#include <cstring>

namespace io {

FdWriter::FdWriter(int fd)
    : fd_(fd){}

FdWriter::~FdWriter() {
    // if (fd_ >= 0) {
    //     ::close(fd_);
    // }
}

FdWriter::WriteResult FdWriter::write(char *buf, std::size_t nbyte) {
    const ssize_t bytesWrite = ::write(fd_, buf, nbyte);
    if (bytesWrite > 0) {
        return OK(static_cast<std::size_t>(bytesWrite));
    }
    return OK(static_cast<std::size_t>(0));
}

FdWriter::WriteResult FdWriter::write(const char *buf, std::size_t nbyte) {
    const ssize_t bytesWrite = ::write(fd_, buf, nbyte);
    if (bytesWrite > 0) {
        return OK(static_cast<std::size_t>(bytesWrite));
    }
    return OK(static_cast<std::size_t>(0));
}
}