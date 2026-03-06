#include <iostream>

#include "../include/Server.h"

int	main(int ac, char **av)
{
    try {
        if (ac > 2)
            throw std::runtime_error("wrong number of arguments");

        std::string config_file_name = ac == 1 ? "default.conf" : av[1];

        if (config_file_name.find(".conf") == std::string::npos)
            throw std::runtime_error("invalid config file format");

        Server server(config_file_name);
		server.run();
 	}
 	catch (SocketException &er) {
 		std::cerr << "Error Socket: " << er.what() << std::endl;
 	}
	catch (ParsingException &er) {
		std::cerr << "Error Parsing: " << er.what() << std::endl;
	}
 	catch (std::exception &er) {
 		std::cerr << "Error: " << er.what() << std::endl;
 	}

 	return (0);
 }

// int main(const int argc, const char** argv) {
// 	if (argc <= 1) {
// 		return 0;
// 	}
// 	try {
// 		GlobalConfig config(argv[1]);
// 		if (DEBUG) {
// 			config.printDirectives();
// 		}
// 	} catch (const std::exception& e) {
// 		std::cout << e.what() << "\n";
// 		return 0;
// 	}
// 	return 0;
// }
