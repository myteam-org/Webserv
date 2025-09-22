#pragma once

class Token;

#include <sys/stat.h>

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "config/token.hpp"

#define FILE_NAME "./config_file/default.conf"

class ConfigTokenizer {
   public:
    explicit ConfigTokenizer(std::string& filename);
    ~ConfigTokenizer();

    const std::vector<Token>& getTokens() const;
    static std::string numberToStr(int number);

   private:
    std::vector<Token> tokens_;

    void makeTokenList_(std::ifstream& file);
    static void checkLineEnd(const std::string& line, int lineCount);
};
