#include "validator.hpp"

#include <unistd.h>

#include <stdexcept>

#include "token.hpp"

bool Validator::number(const std::string& number, int type) {
    for (size_t i = 0; i < number.size(); ++i) {
        if (!isdigit(number[i])) {
            return (false);
        }
    }

    const int num = atoi(number.c_str());

    if (type == LISTEN) {
        return (num >= 0 && num <= MAX_PORT);
    }
    if (type == MAX_SIZE) {
        return (num > 0 && num <= MAX_BODY_SIZE);
    }
    if (type == ERR_PAGE) {
        return (num >= MIN_PAGE_NUM && num <= MAX_PAGE_NUM);
    }
    return (true);
}

bool Validator::isValidIndexFile(const std::string& indexFile) {
    if (indexFile.empty()) {
        return false;
    }
    return indexFile.find(".html") != std::string::npos ||
           indexFile.find(".htm") != std::string::npos;
}

bool Validator::isValidRoot(const std::string& root) {
    if (root.empty()) {
        return false;
    }

    struct stat sta;
    return stat(root.c_str(), &sta) == 0 && S_ISDIR(sta.st_mode) &&
           access(root.c_str(), R_OK | X_OK) == 0;
}
