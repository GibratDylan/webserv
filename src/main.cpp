#include <iostream>

#include "../include/Server.h"
#include "../include/utility/Logger.hpp"

int main(int ac, char** av) {
	Logger::setLevel(Logger::DEBUG);
	Logger::setTimestamps(true);

	try {
		if (ac > 2) throw std::runtime_error("wrong number of arguments");

		std::string config_file_name = ac == 1 ? "default.conf" : av[1];

		if (config_file_name.find(".conf") == std::string::npos) throw std::runtime_error("invalid config file format");

		Server server(config_file_name);
		server.run();
	} catch (SocketException& er) {
		Logger::error(std::string("Socket: ") + er.what());
	} catch (ParsingException& er) {
		Logger::error(std::string("Parsing: ") + er.what());
	} catch (std::exception& er) {
		Logger::error(er.what());
	}

	return (0);
}
