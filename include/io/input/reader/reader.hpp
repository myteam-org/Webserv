#pragma once

#include "utils/types/result.hpp"  // この行を追加
#include "utils/types/error.hpp"     // error::AppErrorを使用している場合

namespace io {
    class IReader {
    public:
        typedef types::Result<std::size_t, error::AppError> ReadResult;

        virtual ~IReader() {}
        
        virtual ReadResult read(char *buf, std::size_t nbyte) = 0;
    };
}
