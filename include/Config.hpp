#pragma once

class ConfigParser;
class ConfigTokenizer;

#include <sys/stat.h>

#include <string>

#include "ConfigNode.hpp"
#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"

#define FILE_NAME "./config_file/default.conf"

class Config {
       private:
        ConfigTokenizer parser_;
        ConfigParser tree_;

       public:
        explicit Config(const std::string& filename);
        ~Config();

        void printTree();
        void printTreeRecursion(ConfigNode* node, int depth = 0);
        ConfigTokenizer& getParser();
        const ConfigParser& getTree() const;
};
