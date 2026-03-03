#include <fstream>
#include <iostream>

#include "../include/webserv.hpp"

std::string getFileContent(const std::string& filename) {
	std::string line;
	std::string file;
	std::ifstream ifs(filename.c_str());

	if (ifs.fail()) {
		throw std::exception();
	}
	while (true) {
		if (!std::getline(ifs, line)) {
			// if (!file.empty()) {
			// 	file.resize(file.size() - 1);
			// }
			return file;
		}
		if (ifs.fail()) {
			throw std::exception();
		}
		file += line + "\n";
	}
}

int main(const int argc, const char** argv) {
	if (argc <= 1) {
		return 0;
	}
	try {
		std::string file(getFileContent(argv[1]));
		GlobalConfig config(file);
		if (DEBUG) {
			config.printDirectives();
		}
	} catch (const std::exception& e) {
		std::cout << "Error\n";
		return 0;
	}
	return 0;
}
