#pragma once

#include <sys/stat.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "ConfigNode.hpp"

enum {
        // DIRECTORY,
        FILENAME
};

namespace Validator {
// bool	validate(const Config& config);
bool number(const std::string& number, int kind);
bool numberAndFile(const std::vector<std::string>& tokens, int i);
bool path(const std::string& path, int select);
bool method(const std::string& method);
bool onOff(const std::string& onOff);
void checkSyntaxErr(const Token& token, int depth);
bool url(const std::string& url);
// bool	duplicate(const ConfigNode* root);
}  // namespace Validator
