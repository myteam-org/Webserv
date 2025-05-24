#include "Config.hpp"

Config::Config(const std::string& filename) {
	_makeToken(filename);
	_makeConfTree(_tokens);
	// checkTree(); TODO
}

Config::~Config() {
	_deleteTree(layers[0]);
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

		char	c = oneLine[oneLine.size() - 1];
		if (!(c == '{' || c == '}' || c == ';' || c == 0))
			throw (std::runtime_error("Syntax error at the end of the line"));

		std::istringstream	tokenStream(oneLine);
		std::string		token;
		while (tokenStream >> token)				// 空白区切りでtokenをset
			this->_tokens.push_back(token);
	}
	file.close();
}

void	Config::_makeConfTree(const std::vector<std::string>& tokens) {
	_init();
	for (size_t i = 0; i < tokens.size(); ++i) {
		int	kind = ConfigNode::tokenKind(tokens[i]);

		_checkSyntaxErr(tokens[i]);
		if (kind == SERVER) {
			ConfigNode::addChild(tokens[i], layers[1], layers[0]);
			_server++;
		} else if (kind == LOCATION) {
			ConfigNode::addChildSetValue(tokens, &i, layers[2], layers[1]);
			_location++;
		} else if (kind == ERR_PAGE) {
			ConfigNode::addChild(tokens[i], layers[3], layers[2]);
		} else if (kind == BRACE) {
			_updateBrace(tokens[i]);
		// error_pageのchildにstatus No.をaddしvalueをsetする
		} else if (_location == 1 && i > 0 && (tokens [i - 1] == "error_page"))
			ConfigNode::addChildSetValue(tokens, &i, layers[4], layers[3]);
		// serverにchildrenとValueをaddする
		else if (_server== 1 && _brace == 1 && (kind >= LISTEN && kind <= RETURN))
			ConfigNode::addChildSetValue(tokens, &i, layers[2], layers[1]);
		// locationにchildrenとvalueをaddする
		else if (_server== 1 && _brace == 2 && _location== 1 && (kind >= LISTEN && kind <= RETURN))
			ConfigNode::addChildSetValue(tokens, &i, layers[3], layers[2]);
		else
			throw (std::runtime_error("Config file syntax error: " + tokens[i]));
	}
}

void	Config::_init() {
	for (int i = 0; i < 5; ++i)
		this->layers.push_back(new ConfigNode("root"));
	this->_server= 0;
	this->_location= 0;
	this->_brace = 0;
}

void	Config::_checkSyntaxErr(std::string token) {
	int	kind = ConfigNode::tokenKind(token);
	if ((kind == SERVER && !(_server== 0 && _brace == 0 && _location == 0)) ||
	    (kind == LOCATION && !(_server== 1 && _brace == 1 && _location == 0)) ||
	    (kind == ERR_PAGE && !(_server== 1 && _brace == 2 && _location== 1))) {
		    throw (std::runtime_error("Syntax error: " + token));
	    }
}

void	Config::_updateBrace(const std::string& token) {
	if (token == "{") {
		_brace++;
	} else if (token == "}") {
		_brace--;
		if (_brace < 0)
			throw (std::runtime_error("4 Config brace close error: " + token));
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

void	Config::printTree(ConfigNode* node, int depth) {
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
		printTree(*it, depth + 1);
}
