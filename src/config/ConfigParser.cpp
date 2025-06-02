#include "ConfigParser.hpp"

#include "Validator.hpp"

ConfigParser::ConfigParser(const ConfigTokenizer& parser) : parser(parser) {
        std::fill(keyFlag_, keyFlag_ + 16, 0);
        makeConfTree_(parser);
}

ConfigParser::~ConfigParser() {
        if (this->root_) {
                deleteTree_(this->root_);
                this->root_ = NULL;
        }
}

ConfigNode* ConfigParser::getRoot() const { return (this->root_); }

void ConfigParser::makeConfTree_(const ConfigTokenizer& parser) {
        this->layers_[0] = new ConfigNode(Token("root", 99));
        this->root_ = this->layers_[0];
        std::vector<Token> tokens = parser.getTokens();

        for (size_t i = 0; i < tokens.size(); ++i) {
                TokenType type = tokens[i].getType();
                const std::string token = tokens[i].getText();
                int depth = this->keyFlag_[BRACE];
                int lineNumber = tokens[i].getLineNumber();
                Validator::checkSyntaxErr(tokens[i], depth);

                if (type == BRACE)
                        updateDepth_(token, lineNumber);
                else if (type >= SERVER && type <= RETURN)
                        addChild_(tokens[i], layers_[depth + 1],
                                  layers_[depth]);
                else if (i > 0 && tokens[i - 1].getType() == ERR_PAGE)
                        addChild_(tokens[i], layers_[depth + 2],
                                  layers_[depth + 1]);
                else if (this->keyFlag_[ERR_PAGE] == 1)
                        setValue_(tokens[i], layers_[depth + 2]);
                else
                        setValue_(tokens[i], layers_[depth + 1]);
        }
}

void ConfigParser::updateDepth_(const std::string& token,
                                const int lineNumber) {
        if (token == "{") {
                this->keyFlag_[BRACE]++;
        } else if (token == "}") {
                this->keyFlag_[BRACE]--;
                if (this->keyFlag_[BRACE] < 0)
                        errExit_(token, ":Config brace close error: line ",
                                 lineNumber);
                if (this->keyFlag_[BRACE] == 0 && keyFlag_[LOCATION] == 0)
                        errExit_(token, ":Config location error: line ",
                                 lineNumber);
                if (this->keyFlag_[BRACE] == 0) resetKeyFlag_(LOCATION);
                if (this->keyFlag_[BRACE] == 0) resetKeyFlag_(SERVER);
        }
}

void ConfigParser::resetKeyFlag_(const int keyType) {
        this->keyFlag_[keyType] = 0;
}

void ConfigParser::addChild_(const Token& token, ConfigNode*& current,
                             ConfigNode* parent) {
        current = new ConfigNode(token);
        parent->getChildren().push_back(current);
        int keyType = token.getType();
        this->keyFlag_[keyType]++;
}

void ConfigParser::setValue_(const Token& token, ConfigNode* node) {
        std::string text = token.getText();
        Validator::checkSyntaxErr(token, this->keyFlag_[BRACE]);

        if (text.size() == 1 && text[0] == ';')
                errExit_(text, ": Can't find value: line ",
                         token.getLineNumber());
        if (text[text.size() - 1] == ';') {
                text = text.substr(0, text.size() - 1);
                if (this->keyFlag_[ERR_PAGE] == 1) resetKeyFlag_(ERR_PAGE);
        }
        node->getValues().push_back(text);
}

void ConfigParser::errExit_(const std::string& str1, const std::string& str2,
                            const int number) {
        std::cerr << str1 << str2 << number << std::endl;
        if (this->root_) {
                deleteTree_(this->root_);
                this->root_ = NULL;
        }
        std::exit(1);
}

void ConfigParser::deleteTree_(ConfigNode* node) {
        if (!node) return;
        for (std::vector<ConfigNode*>::iterator it =
                 node->getChildren().begin();
             it != node->getChildren().end(); ++it)
                deleteTree_(*it);
        delete (node);
        node = NULL;
}
