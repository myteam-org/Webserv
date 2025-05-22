#include "Config.hpp"

Config::Config(const std::string& filename) {
	_checkFile(filename);
	_makeToken(filename);
	_makeConfTree(getTokens());
	// checkTree(); TODO
	_printTree(layers[0]);
	if (_errFlag > 0)
		throw (std::runtime_error("Config file error"));
}

Config::~Config() {
	_deleteTree(layers[0]);
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

void	Config::_makeConfTree(const std::vector<std::string>& tokens) {
	_init();
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (tokens[i] == "server") {
			if (_checkSyntaxErr(SERVER, tokens[i]) == true){
				ConfigNode::addChild(tokens[i], layers[1], layers[0]);
				_server++;
			}
		} else if (tokens[i] == "location" || tokens[i] == "location_back") {
			if (_checkSyntaxErr(LOCATION, tokens[i]) == true) {
				ConfigNode::addChild(tokens[i], layers[2], layers[1]);
				_location++;
			}
		} else if (tokens[i] == "error_page") {
			if (_checkSyntaxErr(ERR_PAGE, tokens[i]) == true)
				ConfigNode::addChild(tokens[i], layers[3], layers[2]);
		} else if (tokens[i] == "{" || tokens[i] == "}") {
			_updateBrace(tokens[i]);
		} else {
			// error_pageのchildにstatus No.をaddしvalueをsetする
			if (_location == 1 && i > 0 && (tokens [i - 1] == "error_page"))
				ConfigNode::addChildSetValue(tokens, &i, layers[4], layers[3]);
			// locationのvalueをsetする
			else if (_location == 1 && i > 0 && (tokens[i - 1] == "location" || tokens[i - 1] == "location_back"))
				ConfigNode::setValue(tokens[i], layers[2], DIRECTORY);
			// serverのchildrenにaddしてvalueをsetする
			else if (_server== 1 && _brace == 1) {
				ConfigNode::addChildSetValue(tokens, &i, layers[2], layers[1]);
			// locationのchildrenにaddしてvalueをsetする
			} else if (_server== 1 && _brace == 2 && _location== 1) {
				ConfigNode::addChildSetValue(tokens, &i, layers[3], layers[2]);
			} else {
				_printErr("Config file syntax error: ", tokens[i]);
			}
		}
	}
}

void	Config::_init() {
	for (int i = 0; i < 5; ++i)
		this->layers.push_back(new ConfigNode("root"));
	this->_server= 0;
	this->_location= 0;
	this->_brace = 0;
	this->_errFlag = 0;
}

int	Config::_checkSyntaxErr(int select, std::string token) {
	if ((select == SERVER && !(_server== 0 && _brace == 0)) ||
	    (select == LOCATION && !(_server== 1 && _brace == 1)) ||
	    (select == ERR_PAGE && !(_server== 1 && _brace == 2 && _location== 1))) {
		    std::cerr << select << " Syntax error: " << token << std::endl;
		    this->_errFlag++;
		    return (false);
	    }
	return (true);
}

void	Config::_printErr(const std::string& msgA, const std::string& msgB) {
	std::cerr << msgA << msgB << std::endl;
	this->_errFlag++;
}

void	Config::_updateBrace(const std::string& token) {
	if (token == "{") {
		_brace++;
	} else if (token == "}") {
		_brace--;
		if (_brace < 0)
			_printErr("4 Config brace close error: ", token);
		if (_brace == 0)
			_server= 0;
		if (_brace == 1)
			_location= 0;
	}
}

void	Config::_deleteTree(ConfigNode* node) {
	if (!node)
		return ;
	for (std::vector<ConfigNode*>::iterator it = node->children.begin(); it != node->children.end(); ++it )
		_deleteTree(*it);
	delete(node);
	node = NULL;
}

void	Config::_printTree(ConfigNode* node, int depth) {
	if (!node)
		return ;

	for (int i = 0; i < depth * 2; ++i)
		std::cout << " ";
	if (depth > 0)
		std::cout << "|- ";
	std::cout << node->key;

	for (std::vector<std::string>::iterator iter = node->values.begin(); iter != node->values.end(); ++iter)
		std::cout << " " << *iter;
	std::cout << std::endl;

	for (std::vector<ConfigNode*>::iterator it = node->children.begin(); it != node->children.end(); ++it)
		_printTree(*it, depth + 1);
}
