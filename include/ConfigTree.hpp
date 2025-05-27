#pragma once

# include <iostream>
# include <vector>
# include <string>
# include <sys/types.h>
# include <sys/stat.h>
# include "Token.hpp"

class ConfigTree {
public:
	std::string			key;
	std::vector<std::string>	values;
	std::vector<ConfigTree*>	children;
	int				keyKind;
	int				valuesKind;

	ConfigTree();
	ConfigTree(std::string key);
	~ConfigTree();

	static void	addChild(const std::string& token, ConfigTree*& current, ConfigTree* parent);
	static void	setValue(const std::string& token, ConfigTree* node, int kind);
	static void	addChildSetValue(const std::vector<Token>& tokens, size_t* i, 
					ConfigTree*& current, ConfigTree* parent);
};
