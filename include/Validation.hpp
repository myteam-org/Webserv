#pragma once

# include <iostream>
# include <sys/stat.h>
# include <stdexcept>
# include <cstdlib>
# include "Config.hpp"
# include "ConfigNode.hpp"

enum {
	// DIRECTORY,
	FILENAME
};

namespace Validation {
	// bool	validate(const Config& config);
	bool	number(const std::string& number, int kind);
	bool	numberAndFile(const std::vector<std::string>& tokens, int i);
	bool	path(const std::string& path, int select);
	bool	method(const std::string& method);
	bool	onOff(const std::string& onOff);
	// bool	url(const std::string& url);
	// bool	duplicate(const ConfigNode* root);
}
