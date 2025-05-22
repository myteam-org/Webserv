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
	void	_checkFile(const std::string& filename);
	void	_makeToken(const std::string& filename);
public:
	std::vector<ConfigNode*>	layers;
	int		server;
	int		location;
	int		brace;
	int		errFlag;

	Config(const std::string& filename);
	~Config();

	const std::vector<std::string>&	getTokens() const;
	void				makeConfTree(const std::vector<std::string>& tokens);
	void				init();
	int				checkSyntaxErr(int select, std::string token);
	void				printErr(const std::string& msgA, const std::string& msgB);
	void				updateBrace(const std::string& token);
	void				printTree(ConfigNode* node, int depth = 0);
	void				deleteTree(ConfigNode* node);
};
