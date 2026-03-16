/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 12:49:46 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/16 12:46:01 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/FileSystem.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "../../include/utility/Logger.hpp"
#include "../../include/utility/ResourceDeleters.hpp"
#include "../../include/utility/TResourceGard.hpp"

bool FileSystem::exists(const std::string& path) {
	struct stat stats = {};
	if (stat(path.c_str(), &stats) == 0) {
		return true;
	}

	Logger::info(std::string(" Path can't be verified or doesn't exist: ") + path);
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
	if (!isFile(path)) {
		return false;
	}

	struct stat stats = {};
	return stat(path.c_str(), &stats) == 0 && static_cast<bool>(stats.st_mode & S_IWUSR);
}

std::string FileSystem::readFile(const std::string& path) {
	if (!isReadable(path)) {
		Logger::info(std::string(" Path is not readable: ") + path);
		throw std::exception();
	}

	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file) {
		Logger::error(std::string(" Failed to open file for read: ") + path);
		throw std::exception();
	}

	std::ostringstream stream;
	stream << file.rdbuf();
	if (!file) {
		Logger::error(std::string(" Failed to read: ") + path);
		throw std::exception();
	}

	return stream.str();
}

std::vector<std::string> FileSystem::listDirectory(const std::string& path) {
	DIR* dir = opendir(path.c_str());
	if (!static_cast<bool>(dir)) {
		Logger::error(std::string(" Failed to get all directory entries in: ") + path);
		throw std::exception();
	}

	typedef TResourceGuard<DIR*, ResourceDeleters::closeDirPointer> DirGuard;
	DirGuard dir_guard(dir);

	std::vector<std::string> files;

	struct dirent* entry = NULL;
	while ((entry = readdir(dir)) != NULL) {
		files.push_back(static_cast<const char*>(entry->d_name));
	}

	std::sort(files.begin(), files.end());

	return files;
}

bool FileSystem::writeFile(const std::string& path, const std::string& content) {
	if (exists(path) && !isWritable(path)) {
		Logger::info(std::string(" Path exists but is not writable: ") + path);
		return false;
	}

	std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);
	if (!file.is_open()) {
		Logger::error(std::string(" Failed to open file for write: ") + path);
		return false;
	}

	file.write(content.c_str(), static_cast<long>(content.size()));
	if (!file) {
		Logger::error(std::string(" Failed to write: ") + path);
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
		Logger::error(std::string(" Failed to delete file: ") + path);
		return false;
	}

	return true;
}
