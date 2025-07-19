#include "string.hpp"

bool utils::startsWith(const std::string &str, const std::string &prefix) {
    return str.find(prefix) == 0;
}

std::string utils::toLower(const std::string &str) {
	std::string result = str;
	for (std::size_t i = 0; i < result.size(); ++i) {
		if ('A' <= result[i] && result[i] <= 'Z') {
			result[i] = static_cast<char>(result[i] - 'A' + 'a');
		}
	}
	return result;
}
