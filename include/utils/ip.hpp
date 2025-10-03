#pragma once

#include <string>

namespace utils {

static const int IP_MAX = 255;
static const int BITS7 = 127;

bool isCanomicalDecimalIPv4(const std::string& ipAddress);
bool isRegularNumber(const std::string& ipAddress, int num[4]);

} // namespace utils
