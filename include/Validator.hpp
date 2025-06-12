#pragma once

class Token;

#include <sys/stat.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

enum {
        // DIRECTORY,
        FILENAME
};

static const int MAX_PORT = 65535;
static const int MAX_BODY_SIZE = 1000000;
static const int MIN_PAGE_NUM = 100;
static const int MAX_PAGE_NUM = 505;

namespace Validator {
bool number(const std::string& number, int type);
bool path(const std::string& path, int select);
bool url(const std::string& url);
// bool	duplicate(const ConfigNode* root);
}  // namespace Validator
