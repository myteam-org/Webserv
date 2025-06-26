#include "validator.hpp"

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

bool Validator::url(const std::string& url) {
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        return (false);
    }
    return (true);
}
