#include "ConfigNode.hpp"

ConfigNode::ConfigNode() {}

ConfigNode::ConfigNode(std::string key) 
	: key(key) {
	// keyKind = tokenKind(key);
}

ConfigNode::~ConfigNode() {}
	
// int	ConfigNode::tokenKind(const std::string& string) {
// 	if (string == "root")
// 		return (ROOT);
// 	if (string == "server")
// 		return (SERVER);
// 	if (string == "location" || string == "location_back")
// 		return (LOCATION);
// 	if (string == "error_page")
// 		return (ERR_PAGE);
// 	if (string == "listen")
// 		return (LISTEN);
// 	if (string == "server_name")
// 		return (SERVER_NAME);
// 	if (string == "allow_method")
// 		return (METHOD);
// 	if (string == "index")
// 		return (INDEX);
// 	if (string == "client_max_body_size")
// 		return (MAX_SIZE);
// 	if (string == "autoindex")
// 		return (AUTOINDEX);
// 	if (string == "is_cgi")
// 		return (IS_CGI);
// 	if (string == "return")
// 		return (RETURN);
// 	if (string == "{" || string == "}")
// 		return (BRACE);
// 	return (VALUE);
// }
	
void	ConfigNode::addChild(const std::string& token, ConfigNode*& current, ConfigNode* parent) {
	current = new ConfigNode(token);
	parent->children.push_back(current);
	
}

void	ConfigNode::setValue(const std::string& token, ConfigNode* node, int kind) {
	if (token.size() == 1 && token[0] == ';')
		throw (std::runtime_error("can't find vlue: " + token));	
	node->values.push_back(token);
	node->valuesKind = kind;
}
	
void	ConfigNode::addChildSetValue(const std::vector<std::string>& tokens, size_t* i,
				     ConfigNode*& current, ConfigNode* parent) {
	Token	t(tokens[*i], 0);
	int	kind = t.getType();
	// int	kind = ConfigNode::tokenKind(tokens[*i]);

	ConfigNode::addChild(tokens[*i], current, parent);
	++*i;
	if (*i >= tokens.size())
		throw (std::runtime_error("no token"));
	if (kind == LOCATION) {
		ConfigNode::setValue(tokens[*i], current, VALUE);
		return ;
	}
	while (*i < tokens.size()) {
		std::string	token = tokens[*i];
		if (token.size() > 1 && token[token.size() - 1] == ';') {
			ConfigNode::setValue(token.substr(0, token.size() - 1), current, VALUE);
			break;
		} else if (token.size() == 1 && token[0] == ';')
			throw (std::runtime_error("can't find vlue: " + token));
		ConfigNode::setValue(tokens[*i], current, VALUE);
		++*i;
	}
}
