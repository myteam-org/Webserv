#include "ConfigTokenizer.hpp"

#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>

ConfigTokenizer::ConfigTokenizer(std::string& filename) {
        std::ifstream file(filename.c_str());
        if (!file) {
                throw(std::runtime_error("Failed to open file: " + filename));
        }
        makeTokenList_(file);
}

ConfigTokenizer::~ConfigTokenizer() {}

const std::vector<Token>& ConfigTokenizer::getTokens() const {
        return (this->tokens_);
}

void ConfigTokenizer::makeTokenList_(std::ifstream& file) {
        std::string line;
        int lineCount = 0;
        while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string oneLine;

                std::getline(iss, oneLine, '#');  // #以降を削除
                oneLine.erase(0, oneLine.find_first_not_of(" \t"));
                oneLine.erase(oneLine.find_last_not_of(" \t") + 1);
                lineCount++;

                checkLineEnd(oneLine, lineCount);

                std::istringstream tokenStream(oneLine);
                std::string token;
                while (tokenStream >> token) {  // 空白区切りでtokenをset
                        const Token newToken(token, lineCount);
                        this->tokens_.push_back(newToken);
                        tokenStream.clear();
                }
        }
        file.close();
}

void ConfigTokenizer::checkLineEnd(const std::string& line,
                                   int lineCount) {
        if (line.empty()) {
                return ;
        }
        const char chara = line[line.size() - 1];
        if (chara != '{' && chara != '}' && chara != ';') {
                throw(std::runtime_error(
                    line + ": Syntax error at the end of the line: line " +
                    numberToStr(lineCount)));
        }
}

std::string ConfigTokenizer::numberToStr(int number) {
        std::stringstream ss;
        ss << number;
        std::string str = ss.str();
        return (str);
}
