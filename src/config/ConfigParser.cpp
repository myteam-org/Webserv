#include "ConfigParser.hpp"
#include <sstream>

ConfigParser::ConfigParser(std::string& filename) { tokenize_(filename); }

ConfigParser::~ConfigParser() {}

const std::vector<Token>& ConfigParser::getTokens() const {
        return (this->tokens_);
}

void ConfigParser::tokenize_(std::string& filename) {
        std::ifstream file(filename.c_str());
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            std::exit(1);
        }

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

                char c = oneLine[oneLine.size() - 1];
                if (!(c == '{' || c == '}' || c == ';' || oneLine.empty())) {
                    std::cerr << "Syntax error at the end of the line: line " << lineCount << std::endl;
                    std::exit(1);
                }

                std::istringstream tokenStream(oneLine);
                std::string token;
                while (tokenStream >> token) {  // 空白区切りでtokenをset
                        Token newToken(token, lineCount);
                        this->tokens_.push_back(newToken);
                }
        }
        file.close();
}
