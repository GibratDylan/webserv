#include "../include/server/Server.hpp"
#include "../include/server/exceptions.hpp"
#include "../include/utility/Logger.hpp"
#include "../include/utility/SignalSystem.hpp"

int main(const int argc, char** argv) {
	Logger::setLevel(Logger::DEBUG);
	Logger::setTimestamps(true);

	try {
		if (argc > 2) {
			throw std::runtime_error("wrong number of arguments: /webserv [configuration file]");
		}

		std::string config_file_name = argc == 1 ? "config/default.conf" : argv[1];

		if (config_file_name.size() < 5 || config_file_name.compare(config_file_name.size() - 5, 5, ".conf") != 0) {
			throw std::runtime_error("invalid config file format");
		}

		SignalSystem::setupSignalSystem();
		Server server(config_file_name);
		server.run();
	} catch (SocketException& er) {
		Logger::error(std::string("Socket: ") + er.what());
		return 1;
	} catch (ParsingException& er) {
		Logger::error(std::string("Parsing: ") + er.what());
		return 1;
	} catch (std::exception& er) {
		Logger::error(er.what());
		return 1;
	}

	return 0;
}
