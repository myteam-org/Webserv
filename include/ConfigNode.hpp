#pragma once

# include <iostream>
# include <vector>
# include <string>
# include <sys/types.h>
# include <sys/stat.h>
# include "Token.hpp"

class ConfigNode {
public:
	ConfigNode();
	ConfigNode(const Token& token);
	// ConfigNode(std::string key);
	~ConfigNode();

	const std::string&		getKey() const;
	std::vector<std::string>&	getValues();
	std::vector<ConfigNode*>&	getChildren();
	TokenType			getKeyKind();
	
	// static void	addChild(const std::string& token, ConfigNode*& current, ConfigNode* parent);
	// static void	setValue(const std::string& token, ConfigNode* node, int kind);
	// static void	addChildSetValue(const std::vector<Token>& tokens, size_t* i, 
	// 				ConfigNode*& current, ConfigNode* parent);
private:	
	std::string			key_;
	std::vector<std::string>	values_;
	std::vector<ConfigNode*>	children_;
	TokenType				keyKind_;
	// int				valuesKind;
};
