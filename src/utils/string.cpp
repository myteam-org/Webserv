#include "string.hpp"

#include <cctype>

bool utils::startsWith(const std::string &str, const std::string &prefix) {
    return str.find(prefix) == 0;
}

std::string utils::toLower(const std::string &str) {
    std::string result = str;
    for (std::size_t i = 0; i < result.size(); ++i) {
        if ('A' <= result[i] && result[i] <= 'Z') {
            result[i] = static_cast<char>(result[i] - 'A' + 'a');
        }
    }
    return result;
}

std::string utils::trim(const std::string &str) {
    std::string::size_type start = 0;
    while (start < str.size() &&
           std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }

    std::string::size_type end = str.size();
    while (end > start &&
           std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }

    return str.substr(start, end - start);
}
