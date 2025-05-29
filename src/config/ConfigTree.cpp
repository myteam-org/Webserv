#include "ConfigTree.hpp"

ConfigTree::ConfigTree(const ConfigParser& parser)
    : parser(parser), depth_(0), location_(0) {
        try {
                makeConfTree_(parser);
        } catch (const std::exception& e) {
                std::cerr << "Config Syntax error" << e.what() << std::endl;
                deleteTree_(this->root_);
                this->root_ = NULL;
                throw;
        }
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
                        updateDepth_(token);
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
                        throw(std::runtime_error("Config file syntax error: " +
                                                 token));
                }
        }
}

void ConfigTree::updateDepth_(const std::string& token) {
        if (token == "{") {
                this->depth_++;
        } else if (token == "}") {
                this->depth_--;
                if (this->depth_ < 0)
                        throw(std::runtime_error(
                            "4 Config brace close error: " + token));
                if (this->depth_ == 0 && location_ == 0)
                        throw(std::runtime_error("Config location error: " +
                                                 token));
        }
}

void ConfigTree::addChild(const Token& token, ConfigNode*& current,
                          ConfigNode* parent) {
        current = new ConfigNode(token);
        parent->getChildren().push_back(current);
        if (token.getType() == SERVER) this->location_ = 0;
}

void ConfigTree::setValue(const std::string& token, ConfigNode* node) {
        if (token.size() == 1 && token[0] == ';')
                throw(std::runtime_error("can't find value: " + token));
        node->getValues().push_back(token);
}

void ConfigTree::addChildSetValue(const std::vector<Token>& tokens, size_t* i,
                                  ConfigNode*& current, ConfigNode* parent) {
        const std::string token = tokens[*i].getText();
        const TokenType kind = tokens[*i].getType();

        ConfigTree::addChild(tokens[*i], current, parent);
        ++*i;
        if (*i >= tokens.size()) throw(std::runtime_error("no token"));
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
                        throw(std::runtime_error("can't find value: " + token));
                        ConfigTree::setValue(token, current);
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
