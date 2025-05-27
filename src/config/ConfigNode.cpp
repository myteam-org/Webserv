#include "ConfigNode.hpp"

ConfigNode::ConfigNode() {}

ConfigNode::ConfigNode(std::string key) 
	: key(key) {
}

ConfigNode::~ConfigNode() {}

void	ConfigNode::addChild(const std::string& token, ConfigNode*& current, ConfigNode* parent) {
	current = new ConfigNode(token);
	parent->children.push_back(current);
	
}

void	ConfigNode::setValue(const std::string& token, ConfigNode* node, int kind) {
	if (token.size() == 1 && token[0] == ';')
		throw (std::runtime_error("can't find vlue: " + token));	
	node->values.push_back(token);
	node->valuesKind = kind;
}
	
void	ConfigNode::addChildSetValue(const std::vector<Token>& tokens, size_t* i,
				     ConfigNode*& current, ConfigNode* parent) {
	const std::string	token = tokens[*i].getText();
	const TokenType		kind = tokens[*i].getType();

	ConfigNode::addChild(token, current, parent);
	++*i;
	if (*i >= tokens.size())
		throw (std::runtime_error("no token"));
	if (kind == LOCATION) {
		ConfigNode::setValue(token, current, VALUE);
		return ;
	}
	while (*i < tokens.size()) {
		const std::string token = tokens[*i].getText();
		if (token.size() > 1 && token[token.size() - 1] == ';') {
			ConfigNode::setValue(token.substr(0, token.size() - 1), current, VALUE);
			break;
		} else if (token.size() == 1 && token[0] == ';')
			throw (std::runtime_error("can't find vlue: " + token));
		ConfigNode::setValue(token, current, VALUE);
		++*i;
	}
}
