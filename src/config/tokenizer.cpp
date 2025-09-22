#include "config/tokenizer.hpp"

#include <fstream>
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
        checkLineEnd(oneLine, lineCount);
        lineCount++;
        std::istringstream tokenStream(oneLine);
        std::string token;
        int stringCount = 0;
        while (tokenStream >> token) {  // 空白区切りでtokenをset
            TokenPosition position = MIDDLE;
            if (token[token.size() - 1] == ';') {
                token = token.substr(0, token.size() - 1);
                position = END;
            }
            if (stringCount == 0) {
                    position = BEGINNING;
            }
            const Token newToken(token, lineCount, position);
            stringCount++;
            this->tokens_.push_back(newToken);
        }
    }
    file.close();
}

void ConfigTokenizer::checkLineEnd(const std::string& line, int lineCount) {
    if (line.empty()) {
        return;
    }
    const char chara = line[line.size() - 1];
    if (chara != '{' && chara != '}' && chara != ';') {
        throw(std::runtime_error(
            line + ": Syntax error at the end of the line: line " +
            numberToStr(lineCount)));
    }
    if (line[0] == '}' && line.size() > 1) {
        throw(std::runtime_error(line + ": Syntax error : line " +
            numberToStr(lineCount)));
    }
}        

std::string ConfigTokenizer::numberToStr(int number) {
    std::stringstream ss;
    ss << number;
    const std::string str = ss.str();
    return (str);
}
