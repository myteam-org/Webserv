#include "Config.hpp"

Config::Config(const std::string& filename) {
	_checkFile(filename);
	_makeToken(filename);
	// for (std::vector<std::string>::iterator it = _tokens.begin(); it != _tokens.end(); ++it)
	// 	std::cout << "token = " << *it << std::endl;
	makeConfTree(getTokens());
	for (std::vector<ConfigNode*>::iterator it = root->children.begin(); it != root->children.end(); ++it) {
		std::cout << (*it)->key << std::endl;
		for (std::vector<ConfigNode*>::iterator ite = (*it)->children.begin(); ite != (*it)->children.end(); ++ite) {
			std::cout << " |- " << (*ite)->key;
			for (std::vector<std::string>::iterator iter = (*ite)->values.begin(); iter != (*ite)->values.end(); ++iter)
				std::cout << "  " << *iter;
			std::cout << std::endl;
			for (std::vector<ConfigNode*>::iterator itera = (*ite)->children.begin(); itera != (*ite)->children.end(); ++itera) {
				std::cout << "   |- " << (*itera)->key;
				for (std::vector<std::string>::iterator iterat = (*itera)->values.begin(); iterat != (*itera)->values.end(); ++iterat)
					std::cout << "  " << *iterat;
				std::cout << std::endl;
			}
		}
	}
}

Config::~Config() {
	deleteNode();
}

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

const std::vector<std::string>&	Config::getTokens() const {
	return (this->_tokens);
}

void	Config::makeConfTree(const std::vector<std::string>& tokens) {
	this->root = new ConfigNode("root");
	ConfigNode*	topLayer;
	ConfigNode*	middleLayer;
	ConfigNode*	bottomLayer;
	this->server = 0;
	this->location = 0;
	this->brace = 0;
	this->errFlag = 0;
	
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (tokens[i] == "server") {
			checkSyntaxErr(SERVER, tokens[i]);
			ConfigNode::setChild(tokens[i], topLayer, root);
			server++;
		} else if (tokens[i] == "location" || tokens[i] == "location_back") {
			checkSyntaxErr(LOCATION, tokens[i]);
			ConfigNode::setChild(tokens[i], middleLayer, topLayer);
			location++;
		} else if (tokens[i] == "{") {
			brace++;
		} else if (tokens[i] == "}") {
			brace--;
			if (brace < 0)
				printErr("4 Config brace close error: ", tokens[i]);
			if (brace == 0)
				server = 0;
			if (brace == 1)
				location = 0;
		} else {
			if (i > 0 && (tokens[i - 1] == "location" || tokens[i - 1] == "location_back"))
				ConfigNode::setValue(tokens[i], middleLayer, DIRECTORY);
			else if (server == 1 && brace == 1) {
				ConfigNode::setChild(tokens[i], middleLayer, topLayer);
				++i;
				for (tokens[i]; tokens[i][tokens[i].size() - 1] != ';'; ++i)
					ConfigNode::setValue(tokens[i], middleLayer, VALUE);
				if (tokens[i][tokens[i].size() - 1] == ';')
					ConfigNode::setValue(tokens[i].substr(0, tokens[i].size() - 1), middleLayer, VALUE);
			} else if (server == 1 && brace == 2 && location == 1) {
				ConfigNode::setChild(tokens[i], bottomLayer, middleLayer);
				++i;
				for (tokens[i]; tokens[i][tokens[i].size() - 1] != ';'; ++i)
					ConfigNode::setValue(tokens[i], bottomLayer, VALUE);
				if (tokens[i][tokens[i].size() - 1] == ';')
					ConfigNode::setValue(tokens[i].substr(0, tokens[i].size() - 1), bottomLayer, VALUE);
			}


		}
		
	}
}

void	Config::checkSyntaxErr(int select, std::string token) {
	if ((select == SERVER && !(server == 0 && brace == 0)) ||
	    (select == LOCATION && !(server == 1 && brace == 1))) {
		    std::cerr << select << " Syntax error: " << token << std::endl;
		    this->errFlag++;
	    }	
}

void	Config::printErr(const std::string& msgA, const std::string& msgB) {
	std::cerr << msgA << msgB << std::endl;
	this->errFlag++;
}

void	Config::deleteNode() {
	for (std::vector<ConfigNode*>::iterator it = root->children.begin(); it != root->children.end(); ++it) {
		for (std::vector<ConfigNode*>::iterator ite = (*it)->children.begin(); ite != (*it)->children.end(); ++ite) {
			delete(*ite);
		}
		(*it)->children.clear();
		delete(*it);
	}
	root->children.clear();
	delete(root);
}
