#include "ConfigNode.hpp"

ConfigNode::ConfigNode(std::string key) 
	: key(key) {
	keyKind = setKind(key);
	if (keyKind == -1)
		std::cerr << "5 config syntax error: " << key << std::endl;
}

ConfigNode::~ConfigNode() {}
	
int	ConfigNode::setKind(const std::string& string) {
	if (string == "server")
		return (SERVER);
	if (string == "location" || string == "location_back")
		return (LOCATION);
	if (string == "listen")
		return (LISTEN);
	if (string == "server_name")
		return (SERVER_NAME);
	if (string == "allow_method")
		return (METHOD);
	if (string == "root")
		return (ROOT);
	if (string == "index")
		return (INDEX);
	if (string == "error_page")
		return (ERR_PAGE);
	if (string == "client_max_body_size")
		return (MAX_SIZE);
	if (string == "autoindex")
		return (AUTOINDEX);
	if (string == "is_cgi")
		return (IS_CGI);
	if (string == "return")
		return (RETURN);
	if (string == "{" || string == "")
		return (BRACE);
	char* endP;
	if (strtod(string.c_str(), &endP) >= 400 && strtod(string.c_str(), &endP) < 600)
		return (ERR_STATUS);
	return (-1);
}
	
void	ConfigNode::addChild(const std::string& token, ConfigNode*& current, ConfigNode* parent) {
	current = new ConfigNode(token);
	// current->parent = parent;
	parent->children.push_back(current);
	
}

void	ConfigNode::setValue(const std::string& token, ConfigNode* node, int kind) {
	node->values.push_back(token);
	node->valuesKind = kind;
}
	
void	ConfigNode::addChildSetValue(const std::vector<std::string>& tokens, size_t* i, ConfigNode*& current, ConfigNode* parent) {
	ConfigNode::addChild(tokens[*i], current, parent);
	++*i;
	while (*i < tokens.size()) {
		std::string	token = tokens[*i];
		if (token[token.size() - 1] == ';') {
			ConfigNode::setValue(token.substr(0, token.size() - 1), current, VALUE);
			break;
		}
		ConfigNode::setValue(tokens[*i], current, VALUE);
		++*i;
	}
}

// 	char*	pEnd;
// 	int	num = std::strtod(port.c_str(), &pEnd);
// 	if (num < 0 || num > 65535)
// 		throw std::runtime_error("Invalid PORT number: " + port);
// }

// void	ConfigNode::judgeHostname(const std::string& hostname) {
// 	(void)hostname;
// }

// void	ConfigNode::judgeDirectory(const std::string& directory) {
// 	struct stat s;

// 	if (stat(directory.c_str(), &s) != 0)
// 		throw std::runtime_error("Failed to stat directory: " + directory);
// 	if (s.st_mode & S_IFDIR)
// 		throw std::runtime_error(directory + " is a directory");
// }
