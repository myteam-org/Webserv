#include "io/reader_fd.hpp"
#include "utils/types/result.hpp"
#include <unistd.h>      // read()
#include <cerrno>        // errno
#include <cstring>       // strerror()

namespace io {

FdReader::FdReader(int fd)
    : fd_(fd), eof_(false) {}

FdReader::~FdReader() {}

ReadResult FdReader::read(char *buf, std::size_t nbyte) {
    if (eof_) {
        return Ok(0ul);
    }

    ssize_t readn = ::read(fd_, buf, nbyte);
    if (readn < 0) {
        return Err(error::kIOUnknown);
    }
    if (readn == 0) {
        eof_ = true;  // EOF 到達
        return Ok(0ul);
    }

    return Ok(static_cast<std::size_t>(readn));
}

bool FdReader::eof() {
    return eof_;
}

} // namespace io
