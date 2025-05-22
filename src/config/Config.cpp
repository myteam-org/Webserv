#include "Config.hpp"

Config::Config(const std::string& filename) {
	_checkFile(filename);
	_makeToken(filename);
	makeConfTree(getTokens());
	printTree(layers[0]);
	if (errFlag > 0)
		throw (std::runtime_error("Config file error"));
}

Config::~Config() {
	deleteTree(layers[0]);
}

void	Config::_checkFile(const std::string& filename) {
	struct stat s;

	if (stat(filename.c_str(), &s) != 0)
		throw std::runtime_error("Failed to stat file: " + filename);
	if (s.st_mode & S_IFDIR)
		throw std::runtime_error(filename + " is a directory");
}

void	Config::_makeToken(const std::string& filename) {
	std::ifstream	file(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + filename);
	
	std::string	line;
	while (std::getline(file, line)) {
		std::istringstream	iss(line);
		std::string		oneLine;

		std::getline(iss, oneLine, '#');		    	// #以降を削除
		oneLine.erase(0, oneLine.find_first_not_of(" \t")); 	// 先頭の空白を削除
		oneLine.erase(oneLine.find_last_not_of(" \t") + 1); 	// 末尾の空白を削除

		std::istringstream	tokenStream(oneLine);
		std::string		token;

		while (tokenStream >> token)				// 空白区切りでtokenをset
			this->_tokens.push_back(token);
	}
	file.close();
}

const std::vector<std::string>&	Config::getTokens() const {
	return (this->_tokens);
}

void	Config::makeConfTree(const std::vector<std::string>& tokens) {
	init();
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (tokens[i] == "server") {
			if (checkSyntaxErr(SERVER, tokens[i]) == true){
				ConfigNode::addChild(tokens[i], layers[1], layers[0]);
				server++;
			}
		} else if (tokens[i] == "location" || tokens[i] == "location_back") {
			if (checkSyntaxErr(LOCATION, tokens[i]) == true) {
				ConfigNode::addChild(tokens[i], layers[2], layers[1]);
				location++;
			}
		} else if (tokens[i] == "error_page") {
			if (checkSyntaxErr(ERR_PAGE, tokens[i]) == true)
				ConfigNode::addChild(tokens[i], layers[3], layers[2]);
		} else if (tokens[i] == "{" || tokens[i] == "}") {
			updateBrace(tokens[i]);
		} else {
			// locationのchildにerror_pageをaddする
			if (i > 0 && (tokens [i - 1] == "error_page"))
				ConfigNode::addChildSetValue(tokens, &i, layers[4], layers[3]);
			// locationのvalueをsetする
			else if (i > 0 && (tokens[i - 1] == "location" || tokens[i - 1] == "location_back"))
				ConfigNode::setValue(tokens[i], layers[2], DIRECTORY);
			// serverのchildrenにaddしてvalueをsetする
			else if (server == 1 && brace == 1) {
				ConfigNode::addChildSetValue(tokens, &i, layers[2], layers[1]);
			// locationのchildrenにaddしてvalueをsetする
			} else if (server == 1 && brace == 2 && location == 1) {
				ConfigNode::addChildSetValue(tokens, &i, layers[3], layers[2]);
			} else {
				printErr("Config file syntax error: ", tokens[i]);
			}
		}
	}
}

void	Config::init() {
	for (int i = 0; i < 5; ++i)
		this->layers.push_back(new ConfigNode("root"));
	this->server = 0;
	this->location = 0;
	this->brace = 0;
	this->errFlag = 0;
}

int	Config::checkSyntaxErr(int select, std::string token) {
	if ((select == SERVER && !(server == 0 && brace == 0)) ||
	    (select == LOCATION && !(server == 1 && brace == 1)) ||
	    (select == ERR_PAGE && !(server == 1 && brace == 2 && location == 1))) {
		    std::cerr << select << " Syntax error: " << token << std::endl;
		    this->errFlag++;
		    return (false);
	    }
	return (true);
}

void	Config::printErr(const std::string& msgA, const std::string& msgB) {
	std::cerr << msgA << msgB << std::endl;
	this->errFlag++;
}

void	Config::updateBrace(const std::string& token) {
	if (token == "{") {
		brace++;
	} else if (token == "}") {
		brace--;
		if (brace < 0)
			printErr("4 Config brace close error: ", token);
		if (brace == 0)
			server = 0;
		if (brace == 1)
			location = 0;
	}
}

void	Config::deleteTree(ConfigNode* node) {
	if (!node)
		return ;
	for (std::vector<ConfigNode*>::iterator it = node->children.begin(); it != node->children.end(); ++it )
		deleteTree(*it);
	delete(node);
	node = NULL;
}

void	Config::printTree(ConfigNode* node, int depth) {
	if (!node)
		return ;

	for (int i = 0; i < depth * 2; ++i)
		std::cout << " ";
	if (depth > 0)
		std::cout << "|- ";
	std::cout << node->key;

	for (std::vector<std::string>::iterator iter = node->values.begin(); iter != node->values.end(); ++iter)
		std::cout << " " << *iter;
	std::cout << std::endl;

	for (std::vector<ConfigNode*>::iterator it = node->children.begin(); it != node->children.end(); ++it)
		printTree(*it, depth + 1);
}
