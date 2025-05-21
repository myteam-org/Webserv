#include "Config.hpp"
#include <signal.h>

void	checkInterpreter() {
	// TODO
}

bool	checkArgc(int argc) {
	if (argc > 2) {
		std::cerr << "Argument error" << std::endl;
		return (false);
	}
	return (true);
}

std::string	setFile(int argc, char **argv) {
	std::string	confFile;
	if (argc == 1)
		confFile = FILE_NAME;
	else
		confFile = argv[1];
	return (confFile);
}

int	main(int argc, char **argv) {
	signal(SIGPIPE, SIG_IGN);
	checkInterpreter(); // TODO
	if (checkArgc(argc) == false)
		return (1);
	std::string	confFile = setFile(argc, argv);

	try {
		Config	config(confFile);
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return (1);
	}
	return (0);
}
