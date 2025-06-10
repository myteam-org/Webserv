#include "Validator.hpp"

#include <stdexcept>

#include "Token.hpp"

// bool	Validation::validate()

bool Validator::number(const std::string& number, int type) {
        char* endP;
        const double num = strtod(number.c_str(), &endP);
        const int intNum = static_cast<const int>(num);

        if ((type == LISTEN || type == MAX_SIZE) && *endP != ';') return (false);
        if (type == ERR_PAGE && *endP) return (false);
        if (type == LISTEN) return (intNum >= 0 && intNum <= MAX_PORT);
        if (type == MAX_SIZE) return (intNum > 0 && intNum <= MAX_BODY_SIZE);
        if (type == ERR_PAGE) return (intNum >= MIN_PAGE_NUM && intNum <= MAX_PAGE_NUM);
        return (true);
}

bool Validator::path(const std::string& path, int select) {
        struct stat sta;

        if (stat(path.c_str(), &sta) != 0) return (false);
        return ((select == DIRECTORY && (sta.st_mode & S_IFDIR)) ||
                (select == FILENAME && (sta.st_mode & S_IFREG)));
}

bool Validator::method(const std::string& method) {
        return (method == "GET" || method == "POST" || method == "DELETE");
}

bool Validator::onOff(const std::string& onOff) {
        return (onOff == "on" || onOff == "off");
}

bool Validator::checkSyntaxErr(const Token& token, int depth) {
        const TokenType type = token.getType();

        if ((type == SERVER && depth != 0) ||
            (type == LOCATION && depth != 1) ||
            (type == ERR_PAGE && depth == 0) ||
            ((type >= LISTEN && type <= REDIRECT) && depth == 0)) {
                    return (false);
            }
        return (true);
}

bool Validator::url(const std::string& url) {
        if (url.find("http://") != 0 && url.find("https://") != 0) return (false);
        return (true);
}
