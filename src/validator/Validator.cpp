#include "Validator.hpp"

#include <stdexcept>

// bool	Validation::validate()

bool Validator::number(const std::string& number, int kind) {
        char* endP;
        int num = strtod(number.c_str(), &endP);

        if (*endP) return (false);
        if (kind == LISTEN) return (num >= 0 && num <= 65535);
        if (kind == MAX_SIZE) return (num > 0 && num < 1000000);
        return (true);
}

bool Validator::numberAndFile(const std::vector<std::string>& tokens, int i) {
        char* endP;
        int num = strtod(tokens[i].c_str(), &endP);

        if (*endP) return (false);
        if (num >= 0 && num < 600) return (true);
        return (false);
}

bool Validator::path(const std::string& path, int select) {
        struct stat s;

        if (stat(path.c_str(), &s) != 0) return (false);
        return ((select == DIRECTORY && (s.st_mode & S_IFDIR)) ||
                (select == FILENAME && (s.st_mode & S_IFREG)));
}

bool Validator::method(const std::string& method) {
        return (method == "GET" || method == "POST" || method == "DELETE");
}

bool Validator::onOff(const std::string& onOff) {
        return (onOff == "on" || onOff == "off");
}

bool Validator::checkSyntaxErr(const Token& token, int depth) {
        TokenType kind = token.getType();
        std::string text = token.getText();

        if ((kind == SERVER && depth != 0) ||
            (kind == LOCATION && depth != 1) ||
            (kind == ERR_PAGE && depth == 0) ||
            ((kind >= LISTEN && kind <= RETURN) && depth == 0))
                return (false);
        return (true);
}

bool Validator::url(const std::string& url) {
        if (!(url.find("http://") == 0 || url.find("https://") == 0))
                return (false);
        return (true);
}
