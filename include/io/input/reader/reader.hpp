#pragma once

#include "result.hpp"
#include "error.hpp"

namespace io {
    class IReader {
    public:
        typedef types::Result<std::size_t, error::AppError> ReadResult;

        virtual ~IReader() {}
        
        virtual ReadResult read(char *buf, std::size_t nbyte) = 0;
		virtual bool eof();
    };
} // namespace io
