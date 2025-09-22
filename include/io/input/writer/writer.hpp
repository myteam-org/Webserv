// IWriter.hpp
#pragma once
#include "utils/types/result.hpp"
#include "utils/types/error.hpp"

namespace io {
class IWriter {
public:
    typedef types::Result<std::size_t, error::AppError> WriteResult;

    virtual ~IWriter() {}
    virtual WriteResult write(const char* buf, std::size_t nbyte) = 0;
};
} // namespace io
