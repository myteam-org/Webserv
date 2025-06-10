#pragma once

#include "ServerContext.hpp"
class ConfigParser;
class ConfigTokenizer;

#include <sys/stat.h>

#include <string>

// #include "ConfigNode.hpp"
#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"

#define FILE_NAME "./config_file/default.conf"

class Config {
       private:
        ConfigTokenizer tokenizer_;
        ConfigParser parser_;

       public:
        explicit Config(const std::string& filename);
        ~Config();
		static bool checkArgc(int argc);
		static std::string setFile(int argc, char** argv);
		static void  checkFile(std::string& filename);
        void printParser();
        void printServer(const std::vector<ServerContext>& server);
        ConfigTokenizer& getTokenizer();
        const ConfigParser& getParser() const;
};
