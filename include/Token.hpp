#pragma once

#include <iostream>
#include <map>

enum TokenType {
        SERVER,
        LOCATION,
        ERR_PAGE,
        LISTEN,
        SERVER_NAME,
        ROOT,
        DIRECTORY,
        METHOD,
        ROOT_DIRECTORY,
        INDEX,
        MAX_SIZE,
        AUTOINDEX,
        IS_CGI,
        RETURN,
        VALUE,
        BRACE
};

class Token {
       public:
        Token(const std::string& text, const int lineNumber);
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
