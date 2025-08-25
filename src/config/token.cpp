#include "config/token.hpp"

#include <string>

#include "parser.hpp"
#include "tokenizer.hpp"

Token::Token(const std::string& text, int lineNumber, TokenPosition position)
    : text_(text), lineNumber_(lineNumber), position_(position) {
    checkString_(text, lineNumber);
    setType_(text);
}

Token::~Token() {}

std::string Token::getText() const { return (this->text_); }

int Token::getLineNumber() const { return (this->lineNumber_); }

TokenType Token::getType() const { return (this->type_); }

TokenPosition Token::getPosition() const { return (this->position_); }

void Token::checkString_(const std::string& text, int lineNumber) {
    if (text.find('{') != std::string::npos && text.size() > 1) {
        throw(std::runtime_error(text + ": Syntax error: " +
                                 ConfigTokenizer::numberToStr(lineNumber)));
    }
    if (text.find('}') != std::string::npos && text.size() > 1) {
        throw(std::runtime_error(text + ": Syntax error: " +
                                 ConfigTokenizer::numberToStr(lineNumber)));
    }
}

void Token::setType_(const std::string& text) {
    static std::map<std::string, TokenType> tokenMap;
    if (tokenMap.empty()) {
        tokenMap.insert(std::make_pair("root", ROOT));
        tokenMap.insert(std::make_pair("server", SERVER));
        tokenMap.insert(std::make_pair("location", LOCATION));
        // tokenMap.insert(std::make_pair("location_back", LOCATION));
        tokenMap.insert(std::make_pair("error_page", ERR_PAGE));
        tokenMap.insert(std::make_pair("listen", LISTEN));
        tokenMap.insert(std::make_pair("host", HOST));
        tokenMap.insert(std::make_pair("server_names", SERVER_NAMES));
        tokenMap.insert(std::make_pair("allow_method", METHOD));
        tokenMap.insert(std::make_pair("index", INDEX));
        tokenMap.insert(std::make_pair("client_max_body_size", MAX_SIZE));
        tokenMap.insert(std::make_pair("autoindex", AUTOINDEX));
        tokenMap.insert(std::make_pair("is_cgi", IS_CGI));
        tokenMap.insert(std::make_pair("redirect", REDIRECT));
        tokenMap.insert(std::make_pair("enable_upload", ENABLE_UPLOAD));
    }
    const std::map<std::string, TokenType>::const_iterator it =
        tokenMap.find(text);
    if (text == "{" || text == "}") {
        this->type_ = BRACE;
    } else if (this->position_ == BEGINNING && it != tokenMap.end()) {
        this->type_ = it->second;
    } else if (this->position_ == BEGINNING) {
        throw(std::runtime_error(
            text + ": Syntax error6 : line " +
            ConfigTokenizer::numberToStr(this->lineNumber_)));
    } else {
        this->type_ = VALUE;
    }
}
