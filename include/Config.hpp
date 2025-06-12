#pragma once

class ConfigParser;
class ConfigTokenizer;
#include "ServerContext.hpp"
#include "LocationContext.hpp"
#include "ConfigParser.hpp"

#include <sys/stat.h>
#include <string>

#define FILE_NAME "./config_file/default.conf"

class Config {
       public:
        explicit Config(const std::string& filename);
        ~Config();

        static bool checkArgc(int argc);
		static std::string setFile(int argc, char** argv);
		static void  checkFile(std::string& filename);
        void printParser() const;
        static void printServer(const std::vector<ServerContext>& server);
        static void printLocation(const ServerContext& server);
        const ConfigParser& getParser() const;

       private:
        ConfigTokenizer tokenizer_;
        ConfigParser parser_;
};
