#include "Config.hpp"
#include "ConfigParser.hpp"

bool Config::checkArgc(int argc) {
        if (argc > 2) {
                std::cerr << "Argument error" << std::endl;
                return (false);
        }
        return (true);
}

std::string Config::setFile(int argc, char** argv) {
        std::string confFile;
        if (argc == 1) {
			confFile = FILE_NAME;
		}
        else {
			confFile = argv[1];
		}
        return (confFile);
}

void  Config::checkFile(std::string& filename) {
        struct stat status;

        if (stat(filename.c_str(), &status) != 0) {
                std::cerr << "Failed to stat file: " << filename << std::endl;
                std::exit(1);
        }
        if (status.st_mode & S_IFDIR) {
                std::cerr << filename << ": is a directory" << std::endl;
                std::exit(1);
        }
}

int main(int argc, char** argv) {
        if (!Config::checkArgc(argc)) return (1);
        std::string confFile = Config::setFile(argc, argv);

		Config::checkFile(confFile);

        Config config(confFile);
        config.printParser();
        return (0);
}
