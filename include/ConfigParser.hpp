#pragma once

class ConfigNode;
class ConfigTokenizer;

#include "ConfigTokenizer.hpp"
#include "Token.hpp"

class ConfigParser {
       public:
        explicit ConfigParser(const ConfigTokenizer& tokenizer);
        ~ConfigParser();

        ConfigTokenizer tokens;
        ConfigNode* getRoot() const;
        static void deleteTree(ConfigNode* node);
        void throwErr(const std::string& str1, const std::string& str2,
                      const int number);

       private:
        int keyFlag_[16];
        ConfigNode* layers_[5];

        void makeConfTree_(const ConfigTokenizer& tokens);
        void updateDepth_(const std::string& token, const int lineNumber);
        void resetKeyFlag_(const int keyType);
        void addChild_(const Token& token, ConfigNode*& current,
                       ConfigNode* parent);
        void setValue_(const Token& token, ConfigNode* node);
};
