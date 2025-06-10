#include "Token.hpp"

Token::Token(const std::string& text, const int lineNumber)
    : text_(text), lineNumber_(lineNumber) {
        setType_(text);
}

Token::~Token() {}

std::string Token::getText() const { return (this->text_); }

int Token::getLineNumber() const { return (this->lineNumber_); }

TokenType Token::getType() const { return (this->type_); }

void Token::setType_(const std::string& text) {
        static std::map<std::string, TokenType> tokenMap;

        if (tokenMap.empty()) {
                tokenMap.insert(std::make_pair("root", ROOT));
                tokenMap.insert(std::make_pair("server", SERVER));
                tokenMap.insert(std::make_pair("location", LOCATION));
                tokenMap.insert(std::make_pair("location_back", LOCATION));
                tokenMap.insert(std::make_pair("error_page", ERR_PAGE));
                tokenMap.insert(std::make_pair("listen", LISTEN));
                tokenMap.insert(std::make_pair("host", HOST));
                tokenMap.insert(std::make_pair("allow_method", METHOD));
                tokenMap.insert(std::make_pair("index", INDEX));
                tokenMap.insert(
                    std::make_pair("client_max_body_size", MAX_SIZE));
                tokenMap.insert(std::make_pair("autoindex", AUTOINDEX));
                tokenMap.insert(std::make_pair("is_cgi", IS_CGI));
                tokenMap.insert(std::make_pair("redirect", REDIRECT));
                tokenMap.insert(std::make_pair("{", BRACE));
                tokenMap.insert(std::make_pair("}", BRACE));
        }
        std::map<std::string, TokenType>::const_iterator it =
            tokenMap.find(text);
        if (it != tokenMap.end()) {
                this->type_ = it->second;
        } else {
                this->type_ = VALUE;
        }
}
