#pragma once

# include <iostream>
# include <vector>
# include <string>
# include <sys/types.h>
# include <sys/stat.h>
# include "Config.hpp"

enum DirectiveKind {
	SERVER,
	LOCATION,
	ROOT,
	LISTEN,
	SERVER_NAME,
	DIRECTORY,
	METHOD,
	ROOT_DIRECTORY,
	INDEX,
	ERR_PAGE,
	MAX_SIZE,
	AUTOINDEX,
	IS_CGI,
	RETURN,
	VALUE,
	IP,
	ERR_STATUS,
	BRACE
};

class ConfigNode {
public:
	std::string			key;
	std::vector<std::string>	values;
	std::vector<ConfigNode*>	children;
	int				keyKind;
	int				valuesKind;
	ConfigNode*			parent;

	ConfigNode(std::string key);
	~ConfigNode();

	static int	setKind(const std::string& string);
	static void	addChild(const std::string& token, ConfigNode*& current, ConfigNode* parent);
	static void	setValue(const std::string& token, ConfigNode* node, int kind);
	static void	addChildSetValue(const std::vector<std::string>& tokens, size_t* i, 
					 ConfigNode*& current, ConfigNode* parent);
	// void	judgePort(const std::string& port);
	// void	judgeHostname(const std::string& hostname);
	// void	judgeDirectory(const std::string& directory);
	// void	judgeSize(const std::string& size);
	// void	judgeMethod(const std::string& method);
	// void	judgeDouble();
	// void	judgeURL(const std::string& url);
};
