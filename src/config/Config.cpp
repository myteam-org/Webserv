#include "Config.hpp"

Config::Config(const std::string& filename)
    : tokenizer_(const_cast<std::string&>(filename)), parser_(tokenizer_) {
        // checkTree(); TODO
}

Config::~Config() {}

ConfigTokenizer& Config::getTokenizer() { return (this->tokenizer_); }

const ConfigParser& Config::getParser() const { return (this->parser_); }

void Config::printParser() { printParserRecursion(Config::getParser().getRoot(), 0); }

void Config::printParserRecursion(ConfigNode* node, int depth) {
        if (!node) return;

        for (int i = 0; i < depth * 2; ++i) std::cout << " ";
        if (depth > 0) std::cout << "|- ";
        std::cout << node->getKey();
        for (std::vector<std::string>::iterator iter =
                 node->getValues().begin();
             iter != node->getValues().end(); ++iter)
                std::cout << " " << *iter;
        std::cout << std::endl;
        for (std::vector<ConfigNode*>::iterator it =
                 node->getChildren().begin();
             it != node->getChildren().end(); ++it)
                printParserRecursion(*it, depth + 1);
}
