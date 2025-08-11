#pragma once

class ConfigParser;
class ConfigTokenizer;
#include <sys/stat.h>

#include <algorithm>
#include <string>

#include "locationContext.hpp"
#include "parser.hpp"
#include "serverContext.hpp"
#include "status.hpp"

#define FILE_NAME "./config_file/default.conf"

typedef std::vector<std::map<http::HttpStatusCode, std::string> > ErrorPageMap;

class Config {
   public:
    explicit Config(const std::string& filename);
    ~Config();

    static bool checkArgc(int argc);
    static std::string setFile(int argc, char** argv);
    static void checkFile(const std::string& filename);
    static bool checkAndEraseLocationNode(const ServerContext& server);
    void checkAndEraseServerNode();
    static void removeDuplicateListenServers(
        std::vector<ServerContext>& servers);
    const ConfigParser& getParser() const;
    void printParser() const;
    static void printServer(const std::vector<ServerContext>& server);
    static void printLocation(const ServerContext& server);

   private:
    ConfigTokenizer tokenizer_;
    ConfigParser parser_;
};
