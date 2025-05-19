#pragma once

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
	std::vector<std::string>	_tokens;
	void	_checkFile(const std::string& filename);
	void	_makeToken(const std::string& filename);
public:
	ConfigNode*	root;
	Config(const std::string& filename);
	~Config();

	makeTree();
};
