#include <iostream>

#include "../include/webserv.hpp"

int main(const int argc, const char** argv) {
	if (argc <= 1) {
		return 0;
	}
	try {
		GlobalConfig config(argv[1]);
		if (DEBUG) {
			config.printDirectives();
		}
	} catch (const std::exception& e) {
		std::cout << e.what() << "\n";
		return 0;
	}
	return 0;
}
