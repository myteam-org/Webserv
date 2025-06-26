#include "io/input/reader/fd.hpp"
#include "io/input/reader/reader.hpp"
#include "utils/types/result.hpp"
#include <unistd.h>
#include <cstring>

namespace io {

FdReader::FdReader(int fd)
    : fd_(fd), eof_(false) {}

FdReader::~FdReader() {}

FdReader::ReadResult FdReader::read(char *buf, std::size_t nbyte) {
    if (eof_) {
        return types::ok(static_cast<std::size_t>(0));
    }

    ssize_t readn = ::read(fd_, buf, nbyte);
    if (readn < 0) {
        return types::err(error::kIOUnknown);
    }
    if (readn == 0) {
        eof_ = true;  // EOF 到達
        return types::ok(static_cast<std::size_t>(0));
    }

    return types::ok(static_cast<std::size_t>(readn));
}

bool FdReader::eof() {
    return eof_;
}

} // namespace io
