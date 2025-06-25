#pragma once

#include "IReader.hpp"
#include <cstddef> // for std::size_t

namespace io {

class FdReader : public IReader {
public:
    explicit FdReader(int fd);
    virtual ~FdReader();

    virtual ReadResult read(char *buf, std::size_t nbyte);
    virtual bool eof();

private:
    int fd_;
    bool eof_;
};

} // namespace io
