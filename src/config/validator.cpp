#include "config/validator.hpp"

#include <unistd.h>

#include <stdexcept>

#include "config/token.hpp"

bool Validator::number(const std::string& number, int type) {
    for (size_t i = 0; i < number.size(); ++i) {
        if (!isdigit(number[i])) {
            return (false);
        }
    }

    const int num = atoi(number.c_str());

    if (type == LISTEN) {
        return (num >= MIN_PORT && num <= MAX_PORT);
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

std::string Validator::dirOf(const std::string& path) {
    if (path.empty()) {
        return ".";
    }
    const std::string::size_type pos = path.rfind('/');
    if (pos == std::string::npos) {
        return ".";
    }
    return pos ? path.substr(0, pos) : "/";
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool Validator::isValidRoot(const std::string& root,
                            const std::string& confPath) {
    struct stat sta;
    std::string path;

    if (!root.empty() && root[0] == '/') {
        path = root;
    } else {
        std::string directory = dirOf(confPath);
        directory = dirOf(directory);
        path = directory + "/" + root;
    }

    return stat(path.c_str(), &sta) == 0 && S_ISDIR(sta.st_mode) &&
           access(path.c_str(), R_OK | X_OK) == 0;
}
