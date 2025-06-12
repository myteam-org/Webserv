#include "Config.hpp"

int main(int argc, char** argv) {
        if (!Config::checkArgc(argc)) return (1);
        std::string confFile = Config::setFile(argc, argv);

        Config::checkFile(confFile);

        try {
                const Config config(confFile);
                config.printParser();
        } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return (1);
        }
        return (0);
}
