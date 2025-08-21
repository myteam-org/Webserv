// IWriter.hpp
#pragma once
#include "result.hpp"
#include "error.hpp"

namespace io {
class IWriter {
public:
    typedef types::Result<std::size_t, error::AppError> WriteResult;

    virtual ~IWriter() {}
    virtual WriteResult write(const char* buf, std::size_t nbyte) = 0;
};
} // namespace io
