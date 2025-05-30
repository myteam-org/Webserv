#include "ConfigNode.hpp"

ConfigNode::ConfigNode() : key_(""), values_(), children_() {}

ConfigNode::ConfigNode(const Token& token)
    : key_(token.getText()), keyKind_(token.getType()) {}

ConfigNode::~ConfigNode() {}

const std::string& ConfigNode::getKey() const { return (this->key_); }

std::vector<std::string>& ConfigNode::getValues() { return (this->values_); }

std::vector<ConfigNode*>& ConfigNode::getChildren() {
        return (this->children_);
}

TokenType ConfigNode::getKeyKind() { return (this->keyKind_); }
