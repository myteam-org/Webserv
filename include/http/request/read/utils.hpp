#pragma once
#include <iostream>
#include <vector>
#include "utils/types/option.hpp"
#include "buffer.hpp"
namespace http {
    typedef std::vector<std::string> RawHeaders;
    typedef types::Result<types::Option<std::string>, error::AppError> GetLineResult;
    GetLineResult getLine(ReadBuffer &readBuf);
}
