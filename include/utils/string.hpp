#pragma once

#include "types/error.hpp"
#include "types/option.hpp"
#include "types/result.hpp"

namespace utils {
bool startsWith(const std::string& str, const std::string& prefix);
std::string toLower(const std::string& str);
std::string trim(const std::string& str);
}  // namespace utils
