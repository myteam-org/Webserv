#pragma once

class Token;

# include "Token.hpp"
# include <iostream>
# include <fstream>
# include <sstream>
# include <string>
# include <sys/stat.h>
# include <stdexcept>
# include <vector>

# define FILE_NAME "./config_file/default.conf"

class ConfigParser {
public:
	ConfigParser(std::string& filename);
	~ConfigParser();

	const std::vector<Token>	getTokens() const;

private:
	void	tokenize_(std::string& filename);
	std::vector<Token>	tokens_;
};
