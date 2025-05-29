#include <signal.h>

#include "Config.hpp"
#include "ConfigTree.hpp"

void checkInterpreter() {
        // TODO
}

bool checkArgc(int argc) {
        if (argc > 2) {
                std::cerr << "Argument error" << std::endl;
                return (false);
        }
        return (true);
}

std::string setFile(int argc, char** argv) {
        std::string confFile;
        if (argc == 1)
                confFile = FILE_NAME;
        else
                confFile = argv[1];
        return (confFile);
}

void checkFile(std::string& filename) {
        struct stat s;

        if (stat(filename.c_str(), &s) != 0)
                throw std::runtime_error("Failed to stat file: " + filename);
        if (s.st_mode & S_IFDIR)
                throw std::runtime_error(filename + " is a directory");
}

int main(int argc, char** argv) {
        signal(SIGPIPE, SIG_IGN);
        checkInterpreter();  // TODO
        if (checkArgc(argc) == false) return (1);
        std::string confFile = setFile(argc, argv);

        try {
                checkFile(confFile);

                Config config(confFile);
                config.printTree(config.getTree().getRoot(), 0);
        } catch (const std::exception& e) {
                return (1);
        }
        return (0);
}
