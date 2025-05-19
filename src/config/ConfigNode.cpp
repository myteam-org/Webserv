#include "ConfigNode.hpp"

ConfigNode::ConfigNode(std::string key, std::string value) : value("") {
	(void)key;
	(void)value;
}

ConfigNode::~ConfigNode() {}

void	ConfigNode::addChild(ConfigNode* child) {
	(void)child;
}

void	ConfigNode::addChild(ConfigNode* child) {

}

void	ConfigNode::judgePort(const std::string& port) {
	int	num = std::strtod(port.c_str());
	if (num < 0 || num > 65535)
		throw std::runtime_error("Invalid PORT number: " + port);
}

void	ConfigNode::judgeHostname(const std::string& hostname) {

}

void	ConfigNode::judgeDirectory(const std::string& directory) {
	struct stat s;

	if (stat(directory.c_str(), &s) != 0)
		throw std::runtime_error("Failed to stat directory: " + directory);
	if (s.st_mode & S_IFDIR)
		throw std::runtime_error(filename + " is a directory");
}