#include "ConfigNode.hpp"

ConfigNode::ConfigNode(std::string key) 
	: key(key) {
	keyKind = setKind(key);
	if (keyKind == -1)
		std::cerr << "config syntax error: " << key << std::endl;
}

ConfigNode::~ConfigNode() {}

void	ConfigNode::addChild(ConfigNode* child) {
	if (!child) {
		std::cerr << "Error: NULL child" << std::endl;
		return ;
	}
	child->parent = this;
	this->children.push_back(child);
}

void	ConfigNode::addValue(const std::string& value) {
	values.push_back(value);
}

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
	return (-1);
}

void	ConfigNode::setChild(const std::string& string, ConfigNode*& current, ConfigNode* parent) {
	current = new ConfigNode(string);
	current->keyKind = setKind(string);
	current->parent = parent;
	parent->children.push_back(current);
}

// void	ConfigNode::judgePort(const std::string& port) {
// 	char*	pEnd;
// 	int	num = std::strtod(port.c_str(), &pEnd);
// 	if (num < 0 || num > 65535)
// 		throw std::runtime_error("Invalid PORT number: " + port);
// }

void	ConfigNode::judgeHostname(const std::string& hostname) {
	(void)hostname;
}

void	ConfigNode::judgeDirectory(const std::string& directory) {
	struct stat s;

	if (stat(directory.c_str(), &s) != 0)
		throw std::runtime_error("Failed to stat directory: " + directory);
	if (s.st_mode & S_IFDIR)
		throw std::runtime_error(directory + " is a directory");
}
