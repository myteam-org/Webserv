#include "Validator.hpp"

// bool	Validation::validate()

bool	Validation::number(const std::string& number, int kind) {
	char*	endP;
	int	num = strtod(number.c_str(), &endP);

	if (*endP)
		throw (std::runtime_error("required number: " + number));
	if (kind == LISTEN)
		return (num >= 0 && num <= 65535);
	if (kind == MAX_SIZE)
		return (num > 0 && num < 1000000);
	return (true);
}

bool	Validation::numberAndFile(const std::vector<std::string>& tokens, int i) {
	char*	endP;
	int	num = strtod(tokens[i].c_str(), &endP);

	if (*endP)
		throw (std::runtime_error("required number: " + tokens[i]));
	// if (tokens[i + 1])
	if (num >= 0 && num < 600)
		return (true);
	return (false);
}

bool	Validation::path(const std::string& path, int select) {
	struct stat	s;

	if (stat(path.c_str(), &s) != 0)
		throw (std::runtime_error("Failed to stat path: " + path));
	return ((select == DIRECTORY && (s.st_mode & S_IFDIR)) ||
		(select == FILENAME && (s.st_mode & S_IFREG)));
}

bool	Validation::method(const std::string& method) {
	return (method == "GET" || method == "POST" || method == "DELETE");
}

bool	Validation::onOff(const std::string& onOff) {
	return (onOff == "on" || onOff == "off");
}

// bool	Validation::url(const std::string& url) {

// }
