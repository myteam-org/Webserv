#pragma once

class ConfigNode;
class ConfigParser;
class Token;

# include "ConfigNode.hpp"
# include "ConfigParser.hpp"
# include "Token.hpp"
# include "Validation.hpp"
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
	void				_makeConfTree(const std::vector<std::string>& tokens);
	void				_init();
	void				_checkSyntaxErr(std::string token);
	void				_updateDepth(const std::string& token);
	void				_deleteTree(ConfigNode* node);
	ConfigParser			parser_;
public:
	Config(const std::string& filename);
	~Config();

	ConfigNode*	layers[5];
	void		printTree(ConfigNode* node, int depth = 0);
};
