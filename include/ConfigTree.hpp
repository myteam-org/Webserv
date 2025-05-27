#pragma once

# include <iostream>
# include "ConfigNode.hpp"
# include "Token.hpp"
# include "ConfigParser.hpp"

class ConfigTree {
public:
	ConfigTree(const ConfigParser& parser);
	~ConfigTree();

	ConfigParser	parser;
	ConfigNode*	getRoot() const;
	void		addChild(const Token& token, ConfigNode*& current, ConfigNode* parent);
	void		setValue(const std::string& token, ConfigNode* node, int type);
	void		addChildSetValue(const std::vector<Token>& tokens, size_t* i, 
					ConfigNode*& current, ConfigNode* parent);
private:
	int		depth_;
	ConfigNode*	root_;
	ConfigNode*	layers_[5];
	// void		makeConfTree(const std::vector<Token>& tokens)
	void		makeConfTree_(const ConfigParser& parser);
	void		checkSyntaxErr_(const Token token);
	void		updateDepth_(const std::string& token);
	void		deleteTree_(ConfigNode* node);
};
