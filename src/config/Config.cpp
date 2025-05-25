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
		if (kind == BRACE)
			_updateBrace(tokens[i]);
		else if (kind == SERVER || kind == ERR_PAGE)
			ConfigNode::addChild(tokens[i], layers[_brace + 1], layers[_brace]);
		else if (i > 0 && (tokens [i - 1] == "error_page"))
			ConfigNode::addChildSetValue(tokens, &i, layers[_brace + 2], layers[_brace + 1]);
		else if (kind == LOCATION || (kind >= LISTEN && kind <= RETURN))
			ConfigNode::addChildSetValue(tokens, &i, layers[_brace + 1], layers[_brace]);
		else
			throw (std::runtime_error("Config file syntax error: " + tokens[i]));
	}
}

void	Config::_init() {
	this->layers[0] = new ConfigNode("root");
	this->_brace = 0;
}

void	Config::_checkSyntaxErr(std::string token) {
	int	kind = ConfigNode::tokenKind(token);
	if ((kind == SERVER && _brace != 0) ||
	    (kind == LOCATION && _brace != 1) ||
	    (kind == ERR_PAGE && _brace == 0) ||
	    ((kind >= LISTEN && kind <= RETURN) && _brace == 0)) {
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
