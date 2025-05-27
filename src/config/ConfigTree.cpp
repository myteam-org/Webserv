#include "ConfigTree.hpp"

ConfigTree::ConfigTree(const ConfigParser& parser) : parser(parser) {
	makeConfTree_(parser);
}

ConfigTree::~ConfigTree() {
	deleteTree_(this->layers_[0]);
}

ConfigNode*	ConfigTree::getRoot() const {
	return (this->root_);
}

// void	Config::_makeConfTree(const std::vector<Token>& tokens) {
void	ConfigTree::makeConfTree_(const ConfigParser& parser) {
	this->layers_[0] = new ConfigNode(Token("root", 0));
	std::vector<Token>	tokens = parser.getTokens();
	this->depth_ = 0;

	for (size_t i = 0; i < tokens.size(); ++i) {
		TokenType	kind = tokens[i].getType();
		const std::string	token = tokens[i].getText();

		checkSyntaxErr_(tokens[i]);
		if (kind == BRACE)
			updateDepth_(token);
		else if (kind == SERVER || kind == ERR_PAGE)
			ConfigTree::addChild(tokens[i], layers_[depth_ + 1], layers_[depth_]);
		else if (i > 0 && (tokens [i - 1].getText() == "error_page")) {
			// Validation::numberAndFile(tokens, i);
			ConfigTree::addChildSetValue(tokens, &i, layers_[depth_ + 2], layers_[depth_ + 1]);
		} else if (kind == LOCATION || (kind >= LISTEN && kind <= RETURN))
			ConfigTree::addChildSetValue(tokens, &i, layers_[depth_ + 1], layers_[depth_]);
		else
			throw (std::runtime_error("Config file syntax error: " + token));
	}
}

void	ConfigTree::checkSyntaxErr_(const Token token) {
	TokenType	kind = token.getType();
	std::string	text = token.getText();

	if ((kind == SERVER && depth_ != 0) ||
	    (kind == LOCATION && depth_ != 1) ||
	    (kind == ERR_PAGE && depth_ == 0) ||
	    ((kind >= LISTEN && kind <= RETURN) && depth_ == 0)) {
		    throw (std::runtime_error("Syntax error: " + text));
	    }
}

void	ConfigTree::updateDepth_(const std::string& token) {
	if (token == "{") {
		depth_++;
	} else if (token == "}") {
		depth_--;
		if (depth_ < 0)
			throw (std::runtime_error("4 Config brace close error: " + token));
	}
}

void	ConfigTree::addChild(const Token& token, ConfigNode*& current, ConfigNode* parent) {
	// std::cout<< "here token = " << token.getText() << std::endl;
	current = new ConfigNode(token);
	if (!parent) {  // 🔥 parent が NULL の場合のエラーハンドリング
		throw std::runtime_error("ConfigTree::addChild() - parent is nullptr");
	    }
	parent->getChildren().push_back(current);
	
}

void	ConfigTree::setValue(const std::string& token, ConfigNode* node, int kind) {
	if (token.size() == 1 && token[0] == ';')
		throw (std::runtime_error("can't find vlue: " + token));	
	node->getValues().push_back(token);
	(void)kind;
	// node->valuesKind = kind;
}
	
void	ConfigTree::addChildSetValue(const std::vector<Token>& tokens, size_t* i,
				     ConfigNode*& current, ConfigNode* parent) {
	const std::string	token = tokens[*i].getText();
	const TokenType		kind = tokens[*i].getType();

	ConfigTree::addChild(tokens[*i], current, parent);
	++*i;
	if (*i >= tokens.size())
		throw (std::runtime_error("no token"));
	if (kind == LOCATION) {
		ConfigTree::setValue(token, current, VALUE);
		return ;
	}
	while (*i < tokens.size()) {
		const std::string token = tokens[*i].getText();
		if (token.size() > 1 && token[token.size() - 1] == ';') {
			ConfigTree::setValue(token.substr(0, token.size() - 1), current, VALUE);
			break;
		} else if (token.size() == 1 && token[0] == ';') {
			throw (std::runtime_error("can't find vlue: " + token));
			ConfigTree::setValue(token, current, VALUE);
		}
		++*i;
	}
}

void	ConfigTree::deleteTree_(ConfigNode* node) {
	if (!node)
		return ;
	for (std::vector<ConfigNode*>::iterator it = node->getChildren().begin(); it != node->getChildren().end(); ++it )
		deleteTree_(*it);
	delete(node);
	node = NULL;
}
