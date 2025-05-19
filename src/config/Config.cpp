#include "Config.hpp"

Config::Config(const std::string& filename) {
	_checkFile(filename);
	_makeToken(filename);
	for (std::vector<std::string>::iterator it = _tokens.begin(); it != _tokens.end(); ++it)
		std::cout << "token = " << *it << std::endl;
}

Config::~Config() {}

void	Config::_checkFile(const std::string& filename) {
	struct stat s;

	if (stat(filename.c_str(), &s) != 0)
		throw std::runtime_error("Failed to stat file: " + filename);
	if (s.st_mode & S_IFDIR)
		throw std::runtime_error(filename + " is a directory");
}

void	Config::_makeToken(const std::string& filename) {
	std::ifstream	file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + filename);
	
	std::string	line;
	while (std::getline(file, line)) {
		std::istringstream	iss(line);
		std::string		oneLine;

		std::getline(iss, oneLine, '#');		    	// #以降を削除
		oneLine.erase(0, oneLine.find_first_not_of(" \t")); 	// 先頭の空白を削除
		oneLine.erase(oneLine.find_last_not_of(" \t") + 1); 	// 末尾の空白を削除

		std::istringstream	tokenStream(oneLine);
		std::string		token;

		while (tokenStream >> token)				// 空白区切りでtokenをset
			this->_tokens.push_back(token);
	}
	file.close();
}
