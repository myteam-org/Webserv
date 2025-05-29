#pragma once

#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <string>
#include <vector>

#include "Token.hpp"

class ConfigNode {
       public:
        ConfigNode();
        explicit ConfigNode(const Token& token);
        ~ConfigNode();

        const std::string& getKey() const;
        std::vector<std::string>& getValues();
        std::vector<ConfigNode*>& getChildren();
        TokenType getKeyKind();

       private:
        std::string key_;
        std::vector<std::string> values_;
        std::vector<ConfigNode*> children_;
        TokenType keyKind_;
};
