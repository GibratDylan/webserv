#include "../include/server/Server.hpp"
#include "../include/server/exceptions.hpp"
#include "../include/utility/Logger.hpp"
#include "../include/utility/SignalSystem.hpp"
#include "../include/network/IOMultiplexer.hpp"

int main(const int argc, char** argv) {
	Logger::setLevel(Logger::DEBUG);
	Logger::setTimestamps(true);

	try {
		if (argc != 2) {
			throw std::runtime_error("wrong number of arguments: /webserv [configuration file]");
		}

		std::string config_file_name = argv[1];

		if (config_file_name.compare(config_file_name.size() - 5, 5, ".conf") != 0) {
			throw std::runtime_error("invalid config file format");
		}

		Server server(config_file_name);
		IOMultiplexer multiplexer;
		SignalSystem::setupSignalSystem();
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
