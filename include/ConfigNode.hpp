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
	~ConfigNode();

	const std::string&		getKey() const;
	std::vector<std::string>&	getValues();
	std::vector<ConfigNode*>&	getChildren();
	TokenType			getKeyKind();
private:	
	std::string			key_;
	std::vector<std::string>	values_;
	std::vector<ConfigNode*>	children_;
	TokenType				keyKind_;
};
