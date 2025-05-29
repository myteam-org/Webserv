#include "ConfigTree.hpp"

ConfigTree::ConfigTree(const ConfigParser& parser)
    : parser(parser), depth_(0), location_(0) {
      makeConfTree_(parser);
}

ConfigTree::~ConfigTree() {
        if (this->root_) {
                deleteTree_(this->root_);
                this->root_ = NULL;
        }
}

ConfigNode* ConfigTree::getRoot() const { return (this->root_); }

void ConfigTree::makeConfTree_(const ConfigParser& parser) {
        this->layers_[0] = new ConfigNode(Token("root", 99));
        this->root_ = this->layers_[0];
        std::vector<Token> tokens = parser.getTokens();

        for (size_t i = 0; i < tokens.size(); ++i) {
                TokenType kind = tokens[i].getType();
                const std::string token = tokens[i].getText();

                Validator::checkSyntaxErr(tokens[i], depth_);
                if (kind == BRACE) {
                        updateDepth_(token, tokens[i].getLineNumber());
                } else if (kind == SERVER || kind == ERR_PAGE) {
                        ConfigTree::addChild(tokens[i], layers_[depth_ + 1],
                                             layers_[depth_]);
                } else if (i > 0 && (tokens[i - 1].getText() == "error_page")) {
                        // Validation::numberAndFile(tokens, i);
                        ConfigTree::addChildSetValue(tokens, &i,
                                                     layers_[depth_ + 2],
                                                     layers_[depth_ + 1]);
                } else if (kind == LOCATION ||
                           (kind >= LISTEN && kind <= RETURN)) {
                        ConfigTree::addChildSetValue(
                            tokens, &i, layers_[depth_ + 1], layers_[depth_]);
                } else {
                        std::cerr << token << ": Config file syntax error: line " << tokens[i].getLineNumber() << std::endl;
                        std::exit(1);
                }
        }
}

void ConfigTree::updateDepth_(const std::string& token,
                              const int lineNumber) {
        if (token == "{") {
                this->depth_++;
        } else if (token == "}") {
                this->depth_--;
                if (this->depth_ < 0) {
                        std::cerr << token << ":Config brace close error: line " << lineNumber << std::endl;
                        std::exit(1);
                }
                if (this->depth_ == 0 && location_ == 0) {
                        std::cerr << token << ":Config location error: line " << lineNumber << std::endl;
                        std::exit(1);
                }
        }
}

void ConfigTree::addChild(const Token& token, ConfigNode*& current,
                          ConfigNode* parent) {
        current = new ConfigNode(token);
        parent->getChildren().push_back(current);
        if (token.getType() == SERVER) this->location_ = 0;
}

void ConfigTree::setValue(const std::string& token, ConfigNode* node) {
        if (token.size() == 1 && token[0] == ';'){
              std::cerr << token << ": Can't find value" << std::endl;
              std::exit(1);
        }
        node->getValues().push_back(token);
}

void ConfigTree::addChildSetValue(const std::vector<Token>& tokens, size_t* i,
                                  ConfigNode*& current, ConfigNode* parent) {
        const std::string token = tokens[*i].getText();
        const TokenType kind = tokens[*i].getType();

        ConfigTree::addChild(tokens[*i], current, parent);
        ++*i;
        if (*i >= tokens.size()) {
                std::cerr << token << ": Config no token error: line " << tokens[*i].getLineNumber() << std::endl;
                std::exit(1);
        }
        if (kind == LOCATION) {
                ConfigTree::setValue(token, current);
                this->location_++;
                return;
        }
        while (*i < tokens.size()) {
                const std::string token = tokens[*i].getText();
                if (token.size() > 1 && token[token.size() - 1] == ';') {
                        ConfigTree::setValue(token.substr(0, token.size() - 1),
                                             current);
                        break;
                } else if (token.size() == 1 && token[0] == ';') {
                        std::cerr << token << ": Config file .. Can't find value: line " << tokens[*i].getLineNumber() << std::endl;
                        std::exit(1);
                }
                ++*i;
        }
}

void ConfigTree::deleteTree_(ConfigNode* node) {
        if (!node) return;
        for (std::vector<ConfigNode*>::iterator it =
                 node->getChildren().begin();
             it != node->getChildren().end(); ++it)
                deleteTree_(*it);
        delete (node);
        node = NULL;
}
