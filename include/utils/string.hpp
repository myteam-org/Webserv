#pragma once

#include "types/error.hpp"
#include "types/option.hpp"
#include "types/result.hpp"
#include <sstream>

namespace utils {
bool startsWith(const std::string& str, const std::string& prefix);

inline bool endsWith(const std::string &str, const std::string &suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(),
                       suffix) == 0;
}

std::string toLower(const std::string& str);
std::string trim(const std::string& str);
types::Result<std::size_t, error::AppError> parseHex(const std::string& hex);  // 16進数を変換する
bool containsNonDigit(const std::string& val);
std::string joinPath(const std::string& leftPath, const std::string& rightPath);
template <class T>
std::string toString(T value) {
    std::stringstream ss;
    ss << value;

    return ss.str();
}



}  // namespace utils
