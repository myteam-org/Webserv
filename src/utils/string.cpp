#include "string.hpp"

#include <cctype>

static const size_t HEX = 16;

bool utils::startsWith(const std::string &str, const std::string &prefix) {
    return str.find(prefix) == 0;
}

std::string utils::toLower(const std::string &str) {
    std::string result = str;
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(result[i])));
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

std::size_t utils::parseHex(const std::string& hex) {
    std::size_t result = 0;

    for (std::size_t i = 0; i < hex.size(); ++i) {
        char chr = hex[i];
        result *= HEX;

        if (chr >= '0' && chr <= '9') {
            result += static_cast<std::size_t>(hex[i] - '0');
        } else if (chr >= 'a' && chr <= 'f') {
            result += static_cast<std::size_t>(hex[i] - 'a');
        } else if (chr >= 'A' && chr <= 'F') {
            result += static_cast<std::size_t>(hex[i] - 'A');
        } else {
            break ;
        }

    }
    return result;
}

