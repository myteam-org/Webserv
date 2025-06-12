#pragma once

#include <iostream>
#include <map>

enum TokenType {
        LISTEN,
        HOST,
        ERR_PAGE,
        MAX_SIZE,
        LOCATION,
        ROOT,
        METHOD,
        INDEX,
        AUTOINDEX,
        IS_CGI,
        REDIRECT,
        SERVER,
        VALUE,
        BRACE
};

class Token {
       public:
        Token(const std::string& text, int lineNumber);
        ~Token();

        std::string getText() const;
        int getLineNumber() const;
        TokenType getType() const;

       private:
        std::string text_;
        int lineNumber_;
        TokenType type_;
        void setType_(const std::string& text);
};
