#pragma once

#include "io/input/read/buffer.hpp"
#include "utils/types/option.hpp"
#include "utils/types/result.hpp"
#include <string>

namespace http {

typedef types::Result<types::Option<std::string>, error::AppError> GetLineResult;

GetLineResult getLine(ReadBuffer& readBuffer);

} // namespace http
