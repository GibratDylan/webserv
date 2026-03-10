/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 12:49:46 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/10 14:26:00 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/FileSystem.hpp"

#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <sstream>

#include "../../include/utility/Logger.hpp"

/*
** --------------------------------- METHODS ----------------------------------
*/

bool FileSystem::exists(const std::string& path) {
	struct stat stats = {};
	if (stat(path.c_str(), &stats) == 0) {
		return true;
	}

	Logger::info(std::string(" Path can't be verify or don't exists: ") + path + " (" + strerror(errno) + ")");
	return false;
}

bool FileSystem::isFile(const std::string& path) {
	if (!exists(path)) {
		return false;
	}

	struct stat stats = {};
	return stat(path.c_str(), &stats) == 0 && S_ISREG(stats.st_mode);
}

bool FileSystem::isDirectory(const std::string& path) {
	if (!exists(path)) {
		return false;
	}

	struct stat stats = {};
	return stat(path.c_str(), &stats) == 0 && S_ISDIR(stats.st_mode);
}

bool FileSystem::isReadable(const std::string& path) {
	if (!isFile(path)) {
		return false;
	}

	struct stat stats = {};
	return stat(path.c_str(), &stats) == 0 && static_cast<bool>(stats.st_mode & S_IRUSR);
}

bool FileSystem::isWritable(const std::string& path) {
	struct stat stats = {};
	if (stat(path.c_str(), &stats) == 0) {
		if (S_ISDIR(stats.st_mode)) 
			return access(path.c_str(), W_OK | X_OK) == 0;
		return access(path.c_str(), W_OK) == 0;
	}

	std::string parent = ".";
	size_t slashPos = path.find_last_of('/');
	if (slashPos == 0) 
		parent = "/";
	else if (slashPos != std::string::npos) 
		parent = path.substr(0, slashPos);

	if (stat(parent.c_str(), &stats) != 0 || !S_ISDIR(stats.st_mode)) 
		return false;

	return access(parent.c_str(), W_OK | X_OK) == 0;
}

std::string FileSystem::readFile(const std::string& path) {
	if (!isReadable(path)) {
		Logger::info(std::string(" Path is not readable: ") + path);
		throw std::exception();
	}

	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file) {
		Logger::error(std::string(" Failed to open file for read: ") + path + " (" + strerror(errno) + ")");
		throw std::exception();
	}

	std::ostringstream stream;
	stream << file.rdbuf();
	file.close();
	if (!file) {
		Logger::error(std::string(" Failed to read or close: ") + path + " (" + strerror(errno) + ")");
		return "";
	}

	return stream.str();
}

bool FileSystem::writeFile(const std::string& path, const std::string& content) {
	if (!isWritable(path)) {
		Logger::info(std::string(" Path is not writable: ") + path);
		return false;
	}

	std::ofstream file(path.c_str(), std::ios::binary);
	if (!file) {
		Logger::error(std::string(" Failed to open file for write: ") + path + " (" + strerror(errno) + ")");
		return false;
	}

	file.write(content.c_str(), static_cast<long>(content.size()));
	file.close();
	if (!file) {
		Logger::error(std::string(" Failed to write or close: ") + path + " (" + strerror(errno) + ")");
		return false;
	}

	return true;
}

bool FileSystem::deleteFile(const std::string& path) {
	if (!isFile(path)) {
		Logger::info(std::string(" Path is not a file: ") + path);
		return false;
	}

	if (std::remove(path.c_str()) == -1) {
		Logger::error(std::string(" Failed to delete file: ") + path + " (" + strerror(errno) + ")");
		return false;
	}

	return true;
}

/* ************************************************************************** */
