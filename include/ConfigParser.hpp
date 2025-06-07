#pragma once

class ConfigNode;
class ConfigTokenizer;

#include "ConfigTokenizer.hpp"
#include "Token.hpp"

class ConfigParser {
       public:
        explicit ConfigParser(const ConfigTokenizer& tokenizer);
        ~ConfigParser();

        ConfigNode* getRoot() const;
        static void deleteTree(ConfigNode* node);
        void throwErr(const std::string& str1, const std::string& str2,
                      const int number);
private:
		static constexpr int kKeyFlagSize = 16;
        static constexpr int kMaxLayerDepth = 5;

        ConfigTokenizer tokens;
        int keyFlag_[kKeyFlagSize];
        ConfigNode* root_;
        ConfigNode* layers_[kMaxLayerDepth];

        void makeConfTree_(const ConfigTokenizer& tokens);
        void updateDepth_(const std::string& token, int lineNumber);
        void resetKeyFlag_(int keyType);
        void addChild_(const Token& token, ConfigNode*& current,
                       ConfigNode* parent);
        void setValue_(const Token& token, ConfigNode* node);
};
