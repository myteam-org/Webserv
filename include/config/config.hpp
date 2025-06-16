#pragma once

class ConfigParser;
class ConfigTokenizer;
#include "server.hpp"
#include "location.hpp"
#include "parser.hpp"

#include <sys/stat.h>
#include <string>
#include <algorithm>

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
        static bool checkAndEraseLocationNode(const ServerContext& server);
        void checkAndEraseServerNode();
        static void removeDuplicateListenServers(std::vector<ServerContext>& servers);

       private:
        ConfigTokenizer tokenizer_;
        ConfigParser parser_;
};
