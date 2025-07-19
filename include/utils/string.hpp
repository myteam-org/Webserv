#pragma once

#include "types/error.hpp"
#include "types/result.hpp"
#include "types/option.hpp"

namespace utils {
    bool startsWith(const std::string &str, const std::string &prefix);
	std::string toLower(const std::string& str);
} //namespace utils
