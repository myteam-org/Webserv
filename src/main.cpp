#include "config/config.hpp"
#include "server/Server.hpp"
#include "utils/logger.hpp"
#include <csignal>


int main(int argc, char** argv) {
        SET_LOG_LEVEL(Logger::kDebug);
        LOG_INFO("Server starting...");
        std::signal(SIGPIPE, SIG_IGN);
        if (!Config::checkArgc(argc)) {
                LOG_ERROR("Invalid number of arguments");
                return (1);
        }
        const std::string confFile = Config::setFile(argc, argv);
        LOG_INFO("Using configuration file: " + confFile);

        Config::checkFile(confFile);

        try {
                const Config config(confFile);
                LOG_INFO("Configuration loaded successfully");
                config.printParser();
                Server server(config.getParser().getServer());
                server.init();
                LOG_INFO("Server initialized");
                server.run();
        } catch (const std::exception& e) {
                LOG_ERROR("An exception occurred: " + std::string(e.what()));
                std::cerr << e.what() << std::endl;
                return (1);
        }
        LOG_INFO("Server shutting down");
        return (0);
}
