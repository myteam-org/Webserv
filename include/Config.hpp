#pragma once

class ConfigTree;
class ConfigParser;
// class Token;

# include "Token.hpp"
# include "ConfigParser.hpp"
# include "ConfigTree.hpp"
# include "ConfigNode.hpp"
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
	ConfigParser			parser_;
	ConfigTree			tree_;
public:
	Config(const std::string& filename);
	~Config();

	void			printTree(ConfigNode* node, int depth = 0);
	ConfigParser&		getParser();
	const ConfigTree&	getTree() const;
};
