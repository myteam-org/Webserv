#pragma once

class ConfigNode;
class ConfigParser;

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

       private:
        int keyFlag_[16];
        ConfigNode* root_;
        ConfigNode* layers_[5];
        void makeConfTree_(const ConfigParser& parser);
        void addChild(const Token& token, ConfigNode*& current,
                      ConfigNode* parent);
        void setValue(const std::string& token, ConfigNode* node);
        void addChildSetValue(const std::vector<Token>& tokens, size_t* i,
                              ConfigNode*& current, ConfigNode* parent);
        void updateDepth_(const std::string& token, const int lineNumber);
        void errExit_(const std::string& str1, const std::string& str2,
                      const int number);
        void deleteTree_(ConfigNode* node);
};
