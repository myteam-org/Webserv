#pragma once

# include <iostream>
# include <vector>
# include <string>

enum NodeKind {
	ROOT,
	SERVER,
	LISTEN,
	SERVER_NAME,
	LOCAION,
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

}

class ConfigNode {
public:
	std::string			key;
	std::string			value;
	std::vector<ConfigNode*>	children;

	ConfigNode(std::string key, std::string value = "");
	~ConfigNode();

	void	addChild(ConfigNode* child);
	void	judgePort(const std::string& port);
	void	judgeHostname(const std::string& hostname);
	void	judgeDirectory(const std::string& directory);
	void	judgeSize(const std::string& size);
	void	judgeMethod(const std::string& method);
	void	judgeDouble();
	void	judgeURL(const std::string& url);
};
