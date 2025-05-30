#include "Config.hpp"

Config::Config(const std::string& filename)
    : parser_(const_cast<std::string&>(filename)), tree_(parser_) {
        // checkTree(); TODO
}

Config::~Config() {}

ConfigParser& Config::getParser() { return (this->parser_); }

const ConfigTree& Config::getTree() const { return (this->tree_); }

void Config::printTree() { printTreeRecursion(Config::getTree().getRoot(), 0); }

void Config::printTreeRecursion(ConfigNode* node, int depth) {
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
                printTreeRecursion(*it, depth + 1);
}
