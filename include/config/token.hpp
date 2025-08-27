#pragma once

#include <map>
#include <ostream>

enum TokenType {
    LISTEN,
    HOST,
    SERVER_NAMES,
    ERR_PAGE,
    MAX_SIZE,
    LOCATION,
    ROOT,
    METHOD,
    INDEX,
    AUTOINDEX,
    IS_CGI,
    REDIRECT,
    ENABLE_UPLOAD,
    SERVER,
    VALUE,
    BRACE
};

enum TokenPosition { BEGINNING, MIDDLE, END };

class Token {
   public:
    Token(const std::string& text, int lineNumber, TokenPosition position);
    ~Token();

    std::string getText() const;
    int getLineNumber() const;
    TokenType getType() const;
    TokenPosition getPosition() const;

   private:
    std::string text_;
    int lineNumber_;
    TokenType type_;
    TokenPosition position_;
    void setType_(const std::string& text);
    static void checkString_(const std::string& text, int lineNumber);
};
