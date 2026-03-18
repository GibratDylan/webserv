#include "../include/config/GlobalConfig.hpp"
#include "../include/server/Server.hpp"
#include "../include/server/exceptions.hpp"
#include "../include/utility/Logger.hpp"
#include "../include/utility/SignalSystem.hpp"

int main(const int argc, char** argv) {
	Logger::setLevel(Logger::INFO);
	Logger::setTimestamps(true);

	try {
		if (argc > 3) {
			throw std::runtime_error(
				"wrong number of arguments: ./webserv [configuration file] | "
				"./webserv --check-config [configuration file]");
		}

		const bool checkOnly =
			(argc >= 2 && std::string(argv[1]) == "--check-config");
		std::string config_file_name;
		if (checkOnly) {
			config_file_name = (argc == 3) ? argv[2] : "config/default.conf";
		} else {
			config_file_name = (argc == 1) ? "config/default.conf" : argv[1];
		}

		if (config_file_name.size() < 5 ||
			config_file_name.compare(config_file_name.size() - 5, 5, ".conf") !=
				0) {
			throw std::runtime_error("invalid config file format");
		}

		if (checkOnly) {
			GlobalConfig config(config_file_name);
			static_cast<void>(config);
			return 0;
		}

		SignalSystem::setupSignalSystem();
		Server server(config_file_name);
		server.run();
	} catch (const SocketException& er) {
		Logger::error(std::string("Socket: ") + er.what());
		return 1;
	} catch (const ParsingException& er) {
		Logger::error(std::string("Parsing: ") + er.what());
		return 1;
	} catch (const ExecveException& er) {
		// Logger::error(std::string("Parsing: ") + er.what());
		return 1;
	} catch (const std::exception& er) {
		Logger::error(er.what());
		return 1;
	}

	return 0;
}
