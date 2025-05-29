#include "ConfigParser.hpp"
#include <sstream>

// #include <stdexcept>

ConfigParser::ConfigParser(std::string& filename) { tokenize_(filename); }

ConfigParser::~ConfigParser() {}

const std::vector<Token>& ConfigParser::getTokens() const {
        return (this->tokens_);
}

void ConfigParser::tokenize_(std::string& filename) {
        std::ifstream file(filename.c_str());
        if (!file.is_open())
                throw std::runtime_error("Failed to open file: " + filename);

        std::string line;
        int lineCount = 0;
        while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string oneLine;

                std::getline(iss, oneLine, '#');  // #以降を削除
                oneLine.erase(
                    0, oneLine.find_first_not_of(" \t"));  // 先頭の空白を削除
                oneLine.erase(oneLine.find_last_not_of(" \t") +
                              1);  // 末尾の空白を削除
                lineCount++;

                std::ostringstream  oss;
                oss << lineCount;
                char c = oneLine[oneLine.size() - 1];
                if (!(c == '{' || c == '}' || c == ';' || oneLine.empty()))
                        throw(std::runtime_error(
                            "Syntax error at the end of the line: line: line " + oss.str()));

                std::istringstream tokenStream(oneLine);
                std::string token;
                while (tokenStream >> token) {  // 空白区切りでtokenをset
                        Token newToken(token, oss.str());
                        this->tokens_.push_back(newToken);
                }
        }
        file.close();
}
