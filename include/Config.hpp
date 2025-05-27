#pragma once

class ConfigTree;
class ConfigParser;
class Token;

# include "ConfigTree.hpp"
# include "ConfigParser.hpp"
# include "Token.hpp"
# include "Validator.hpp"
# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <sys/stat.h>
# include <stdexcept>
# include <vector>

# define FILE_NAME "./config_file/default.conf"

class Config {
private:
	std::vector<std::string>	_tokens;
	int				_depth;
	// void				_makeToken(const std::string& filename);
	// void				_makeConfTree(const std::vector<std::string>& tokens);
	void				_makeConfTree(const std::vector<Token>& tokens);
	void				_init();
	// void				_checkSyntaxErr(std::string token);
	void				_checkSyntaxErr(const Token token);
	void				_updateDepth(const std::string& token);
	// void				_deleteTree(ConfigTree* node);
	void				_deleteTree(ConfigParser parser_);
	ConfigParser			parser_;
public:
	Config(const std::string& filename);
	~Config();

	ConfigTree*	layers[5];
	void		printTree(ConfigTree* node, int depth = 0);
};
