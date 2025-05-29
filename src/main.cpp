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

        if (stat(filename.c_str(), &s) != 0) {
            std::cerr << "Failed to stat file: " << filename << std::endl;
            std::exit(1);
        }
        if (s.st_mode & S_IFDIR) {
            std::cerr << filename << ": is a directory" << std::endl;
            std::exit(1);
        }
}

int main(int argc, char** argv) {
        signal(SIGPIPE, SIG_IGN);
        checkInterpreter();  // TODO
        if (checkArgc(argc) == false) return (1);
        std::string confFile = setFile(argc, argv);

        checkFile(confFile);

        Config config(confFile);
        config.printTree(config.getTree().getRoot(), 0);
        return (0);
}
