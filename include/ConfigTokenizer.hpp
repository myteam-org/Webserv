#pragma once

class Token;

#include <sys/stat.h>

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "Token.hpp"

#define FILE_NAME "./config_file/default.conf"

class ConfigTokenizer {
       public:
        explicit ConfigTokenizer(std::string& filename);
        ~ConfigTokenizer();

        const std::vector<Token>& getTokens() const;
        static std::string numberToStr(int number);

       private:
        void makeTokenList_(std::ifstream& file);
        void checkLineEnd(const std::string& line, int lineCount);
        std::vector<Token> tokens_;
};
