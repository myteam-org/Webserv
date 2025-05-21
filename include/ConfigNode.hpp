#pragma once

# include <iostream>
# include <vector>
# include <string>
# include <sys/types.h>
# include <sys/stat.h>
# include "Config.hpp"


// enum BlockKind {
// };

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
	NUMBER,
	IP
};

class ConfigNode {
public:
	std::string			key;
	std::vector<std::string>	values;
	std::vector<ConfigNode*>	children;
	int				keyKind;
	int				valuesKind;

	ConfigNode(std::string key);
	~ConfigNode();

	void	addChild(ConfigNode* child);
	void	addValue(const std::string& value);
	static int	setKind(const std::string& string);
	static void	setNode(const std::string& string, ConfigNode* current, 
			        ConfigNode* root, std::vector<ConfigNode*> keep);
	void	judgePort(const std::string& port);
	void	judgeHostname(const std::string& hostname);
	void	judgeDirectory(const std::string& directory);
	// void	judgeSize(const std::string& size);
	// void	judgeMethod(const std::string& method);
	// void	judgeDouble();
	// void	judgeURL(const std::string& url);
};
