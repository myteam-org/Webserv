#pragma once

class ConfigNode;
class ConfigParser;

#include <iostream>

#include "ConfigNode.hpp"
#include "ConfigParser.hpp"
#include "Token.hpp"
#include "Validator.hpp"

class ConfigTree {
       public:
        explicit ConfigTree(const ConfigParser& parser);
        ~ConfigTree();

        ConfigParser parser;
        ConfigNode* getRoot() const;
        void addChild(const Token& token, ConfigNode*& current,
                      ConfigNode* parent);
        void setValue(const std::string& token, ConfigNode* node);
        void addChildSetValue(const std::vector<Token>& tokens, size_t* i,
                              ConfigNode*& current, ConfigNode* parent);

       private:
        int depth_;
        int location_;
        ConfigNode* root_;
        ConfigNode* layers_[5];
        void makeConfTree_(const ConfigParser& parser);
        void updateDepth_(const std::string& token);
        void deleteTree_(ConfigNode* node);
};
