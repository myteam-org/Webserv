#include "ConfigTree.hpp"

ConfigTree::ConfigTree() {}

ConfigTree::ConfigTree(std::string key) 
	: key(key) {
}

ConfigTree::~ConfigTree() {}

void	ConfigTree::addChild(const std::string& token, ConfigTree*& current, ConfigTree* parent) {
	current = new ConfigTree(token);
	parent->children.push_back(current);
	
}

void	ConfigTree::setValue(const std::string& token, ConfigTree* node, int kind) {
	if (token.size() == 1 && token[0] == ';')
		throw (std::runtime_error("can't find vlue: " + token));	
	node->values.push_back(token);
	node->valuesKind = kind;
}
	
void	ConfigTree::addChildSetValue(const std::vector<Token>& tokens, size_t* i,
				     ConfigTree*& current, ConfigTree* parent) {
	const std::string	token = tokens[*i].getText();
	const TokenType		kind = tokens[*i].getType();

	ConfigTree::addChild(token, current, parent);
	++*i;
	if (*i >= tokens.size())
		throw (std::runtime_error("no token"));
	if (kind == LOCATION) {
		ConfigTree::setValue(token, current, VALUE);
		return ;
	}
	while (*i < tokens.size()) {
		const std::string token = tokens[*i].getText();
		if (token.size() > 1 && token[token.size() - 1] == ';') {
			ConfigTree::setValue(token.substr(0, token.size() - 1), current, VALUE);
			break;
		} else if (token.size() == 1 && token[0] == ';') {
			throw (std::runtime_error("can't find vlue: " + token));
			ConfigTree::setValue(token, current, VALUE);
		}
		++*i;
	}
}
