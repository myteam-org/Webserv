#include "config/config.hpp"
#include "server/Server.hpp"

int main(int argc, char** argv) {
        if (!Config::checkArgc(argc)) {
                return (1);
        }
        const std::string confFile = Config::setFile(argc, argv);

        Config::checkFile(confFile);

        try {
                const Config config(confFile);
                config.printParser();
                Server server(config.getParser().getServer());
                server.init();
                server.run();
        } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return (1);
        }
        return (0);
}
