#pragma once

#include "types/error.hpp"
#include "types/option.hpp"
#include "types/result.hpp"
#include <sstream>

namespace utils {
    template <class T>
    std::string toString(T value) {
        std::stringstream ss;
        ss << value;

        return ss.str();
    }

    bool startsWith(const std::string& str, const std::string& prefix);
    bool endsWith(const std::string &str, const std::string &suffix);
    std::string toLower(const std::string& str);
    std::string trim(const std::string& str);
    types::Result<std::size_t, error::AppError> parseHex(const std::string& hex);  // 16進数を変換する
}  // namespace utils
