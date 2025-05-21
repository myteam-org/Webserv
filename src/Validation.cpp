#include "Validation.hpp"

// bool	Validation::validate()

// bool	Validation::brace(const Config& config) {
// 	int	openBrace = 0, closeBrace = 0;
// 	std::vector<std::string> tokens = config.getTokens();

// 	for (size_t i = 0; i < tokens.size(); ++i) {
// 		if (tokens[i] == "{")
// 			openBrace++;
// 		if (tokens[i] == "}")
// 			closeBrace++;
// 	}
// 	return (openBrace = closeBrace);
// }

bool	Validation::number(const std::string& number, int kind) {
	char*	endP;
	int	num = strtod(number.c_str(), &endP);

	if (kind == LISTEN)
		return (num >= 0 && num <= 35535);
	if (kind == IP)
		return (num >= 0 && num <= 255);
	if (kind == MAX_SIZE)
		return (num > 0 && num < 1000000);
}

bool	Validation::path(const std::string& path, int select) {
	struct stat	s;

	if (stat(path.c_str(), &s) != 0)
		throw (std::runtime_error("Failed to stat path: " + path));
	return (select == DIRECTORY && (s.st_mode & S_IFDIR));
	return (select == FILENAME && (s.st_mode & S_IFREG));
	throw (std::runtime_error("Invalid path or file name: " + path));
}

bool	Validation::method(const std::string& method) {
	return (method == "GET" || method == "POST" || method == "DELETE");
}

bool	Validation::onOff(const std::string& onOff) {
	return (onOff == "on" || onOff == "off");
}

bool	Validation::url(const std::string& url) {

}
