#pragma once

# include <iostream>
# include <sys/stat.h>
# include <stdexcept>
# include "Config.hpp"
# include "ConfigNode.hpp"

enum {
	// DIRECTORY,
	FILENAME
};

namespace Validation {
	bool	validate(const Config& config);
	bool	brace(const Config& config);
	bool	hierarchy(const ConfigNode& root);
	bool	number(const std::string& number, int kind);
	bool	path(const std::string& path, int select);
	bool	method(const std::string& method);
	bool	onOff(const std::string& onOff);
	bool	url(const std::string& url);
	bool	duplicate(const ConfigNode* root);
}
