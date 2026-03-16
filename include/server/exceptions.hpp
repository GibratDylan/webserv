#pragma once

#include <stdexcept>

class SocketException : public std::runtime_error {
   public:
	SocketException(std::string msg) : std::runtime_error(msg) {}
};

class ParsingException : public std::runtime_error {
   public:
	ParsingException(std::string msg) : std::runtime_error(msg) {}
};

class ExecveException : public std::exception {
   public:
	ExecveException() {}
};
