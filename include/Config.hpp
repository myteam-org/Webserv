#pragma once

class ConfigTree;
class ConfigParser;

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ConfigNode.hpp"
#include "ConfigParser.hpp"
#include "ConfigTree.hpp"
#include "Token.hpp"

#define FILE_NAME "./config_file/default.conf"

class Config {
       private:
        ConfigParser parser_;
        ConfigTree tree_;

       public:
        explicit Config(const std::string& filename);
        ~Config();

        void printTree(ConfigNode* node, int depth = 0);
        ConfigParser& getParser();
        const ConfigTree& getTree() const;
};
