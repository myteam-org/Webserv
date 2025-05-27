#include "Config.hpp"

Config::Config(const std::string& filename)
	: parser_(const_cast<std::string&>(filename)), tree_(parser_) {
	// for (std::vector<Token>::const_iterator it = parser_.getTokens().begin(); it != parser_.getTokens().end(); ++it)
	// 	std::cout << "token = " << it->getText() << "' kind = " << it->getType() << std::endl;
	// _makeConfTree(parser_.getTokens());
	// checkTree(); TODO
}

Config::~Config() {}

ConfigParser&	Config::getParser() {
	return (this->parser_);
}

const ConfigTree&	Config::getTree() const {
	return (this->tree_);
}

void	Config::printTree(ConfigNode* node, int depth) {
	if (!node)
	return ;
	
	for (int i = 0; i < depth * 2; ++i)
	std::cout << " ";
	if (depth > 0)
	std::cout << "|- ";
	std::cout << node->getKey();
	for (std::vector<std::string>::iterator iter = node->getValues().begin(); iter != node->getValues().end(); ++iter)
	std::cout << " " << *iter;
	std::cout << std::endl;
	for (std::vector<ConfigNode*>::iterator it = node->getChildren().begin(); it != node->getChildren().end(); ++it)
	printTree(*it, depth + 1);
}

// void	Config::_makeConfTree(const std::vector<Token>& tokens) {
// 	_init();
// 	for (size_t i = 0; i < tokens.size(); ++i) {
// 		TokenType	kind = tokens[i].getType();
// 		const std::string	token = tokens[i].getText();

// 		_checkSyntaxErr(tokens[i]);
// 		if (kind == BRACE)
// 			_updateDepth(token);
// 		else if (kind == SERVER || kind == ERR_PAGE)
// 			ConfigNode::addChild(token, layers[_depth + 1], layers[_depth]);
// 		else if (i > 0 && (tokens [i - 1].getText() == "error_page")) {
// 			// Validation::numberAndFile(tokens, i);
// 			ConfigNode::addChildSetValue(tokens, &i, layers[_depth + 2], layers[_depth + 1]);
// 		} else if (kind == LOCATION || (kind >= LISTEN && kind <= RETURN))
// 			ConfigNode::addChildSetValue(tokens, &i, layers[_depth + 1], layers[_depth]);
// 		else
// 			throw (std::runtime_error("Config file syntax error: " + token));
// 	}
// }

// void	Config::_init() {
// 	this->layers[0] = new ConfigNode("root");
// 	this->_depth = 0;
// }

// void	Config::_checkSyntaxErr(const Token token) {
// 	TokenType	kind = token.getType();
// 	std::string	text = token.getText();

// 	if ((kind == SERVER && _depth != 0) ||
// 	    (kind == LOCATION && _depth != 1) ||
// 	    (kind == ERR_PAGE && _depth == 0) ||
// 	    ((kind >= LISTEN && kind <= RETURN) && _depth == 0)) {
// 		    throw (std::runtime_error("Syntax error: " + text));
// 	    }
// }

// void	Config::_updateDepth(const std::string& token) {
// 	if (token == "{") {
// 		_depth++;
// 	} else if (token == "}") {
// 		_depth--;
// 		if (_depth < 0)
// 			throw (std::runtime_error("4 Config brace close error: " + token));
// 	}
// }

// void	Config::_deleteTree(ConfigNode* node) {
// 	if (!node)
// 		return ;
// 	for (std::vector<ConfigNode*>::iterator it = node->children.begin(); it != node->children.end(); ++it )
// 		_deleteTree(*it);
// 	delete(node);
// 	node = NULL;
// }
