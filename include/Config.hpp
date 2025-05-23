#pragma once

class ConfigNode;

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
	int				_server;
	int				_location;
	int				_brace;
	void				_checkFile(const std::string& filename);
	void				_makeToken(const std::string& filename);
	void				_makeConfTree(const std::vector<std::string>& tokens);
	void				_init();
	void				_checkSyntaxErr(int select, std::string token);
	void				_updateBrace(const std::string& token);
	void				_deleteTree(ConfigNode* node);
	void				_printTree(ConfigNode* node, int depth = 0);
public:
	std::vector<ConfigNode*>	layers;

	Config(const std::string& filename);
	~Config();

	const std::vector<std::string>&	getTokens() const;
};
