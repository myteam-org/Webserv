#include "Config.hpp"

Config::Config(const std::string& filename) 
	: parser_(const_cast<std::string&>(filename)) {
	// parser_(filename);
	// _makeToken(filename);
	_makeConfTree(parser_.getTokens());
	// _makeConfTree(_tokens);
	// checkTree(); TODO
}

Config::~Config() {
	_deleteTree(layers[0]);
}

// void	Config::_makeToken(const std::string& filename) {
// 	std::ifstream	file(filename.c_str());
// 	if (!file.is_open())
// 		throw std::runtime_error("Failed to open file: " + filename);
	
// 	std::string	line;
// 	while (std::getline(file, line)) {
// 		std::istringstream	iss(line);
// 		std::string		oneLine;

// 		std::getline(iss, oneLine, '#');		    	// #以降を削除
// 		oneLine.erase(0, oneLine.find_first_not_of(" \t")); 	// 先頭の空白を削除
// 		oneLine.erase(oneLine.find_last_not_of(" \t") + 1); 	// 末尾の空白を削除

// 		char	c = oneLine[oneLine.size() - 1];
// 		if (!(c == '{' || c == '}' || c == ';' || c == 0))
// 			throw (std::runtime_error("Syntax error at the end of the line"));

// 		std::istringstream	tokenStream(oneLine);
// 		std::string		token;
// 		while (tokenStream >> token)				// 空白区切りでtokenをset
// 			this->_tokens.push_back(token);
// 	}
// 	file.close();
// }

// void	Config::_makeConfTree(const std::vector<std::string>& tokens) {
void	Config::_makeConfTree(const std::vector<std::string>& tokens) {
	_init();
	for (size_t i = 0; i < tokens.size(); ++i) {
		Token		t(tokens[i], 0);
		int		kind = t.getType();
		std::string	token = t.getText();

		_checkSyntaxErr(tokens[i]);
		if (kind == BRACE)
			_updateDepth(tokens[i]);
		else if (kind == SERVER || kind == ERR_PAGE)
			ConfigNode::addChild(tokens[i], layers[_depth + 1], layers[_depth]);
		else if (i > 0 && (tokens [i - 1] == "error_page")) {
			Validation::numberAndFile(tokens, i);
			ConfigNode::addChildSetValue(tokens, &i, layers[_depth + 2], layers[_depth + 1]);
		} else if (kind == LOCATION || (kind >= LISTEN && kind <= RETURN))
			ConfigNode::addChildSetValue(tokens, &i, layers[_depth + 1], layers[_depth]);
		else
			throw (std::runtime_error("Config file syntax error: " + tokens[i]));
	}
}

void	Config::_init() {
	this->layers[0] = new ConfigNode("root");
	this->_depth = 0;
}

void	Config::_checkSyntaxErr(std::string token) {
	Token	t(token, 0);
	int	kind = t.getType();
	if ((kind == SERVER && _depth != 0) ||
	    (kind == LOCATION && _depth != 1) ||
	    (kind == ERR_PAGE && _depth == 0) ||
	    ((kind >= LISTEN && kind <= RETURN) && _depth == 0)) {
		    throw (std::runtime_error("Syntax error: " + token));
	    }
}

void	Config::_updateDepth(const std::string& token) {
	if (token == "{") {
		_depth++;
	} else if (token == "}") {
		_depth--;
		if (_depth < 0)
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
