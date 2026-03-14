/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 12:49:46 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/11 20:22:18 by dgibrat          ###   ########.fr       */
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

/*
** --------------------------------- METHODS ----------------------------------
*/

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
	struct stat stats = {};
	if (stat(path.c_str(), &stats) == 0) {
		if (S_ISDIR(stats.st_mode)) return access(path.c_str(), W_OK | X_OK) == 0;
		return access(path.c_str(), W_OK) == 0;
	}

	std::string parent = ".";
	size_t slashPos = path.find_last_of('/');
	if (slashPos == 0)
		parent = "/";
	else if (slashPos != std::string::npos)
		parent = path.substr(0, slashPos);

	if (stat(parent.c_str(), &stats) != 0 || !S_ISDIR(stats.st_mode)) return false;

	return access(parent.c_str(), W_OK | X_OK) == 0;
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

	typedef TResourceGuard<std::ifstream&, ResourceDeleters::closeInFileStream> ifstreamGuard;
	ifstreamGuard ifstream_guard(file);

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

bool FileSystem::findIndexFile(const std::string& dir, const std::vector<std::string>& indexes,
							   std::string& indexPath, std::string& indexName) {
	for (size_t i = 0; i < indexes.size(); ++i) {
		std::string path = dir;
		if (!path.empty() && path[path.size() - 1] != '/') 
			path += "/";
		
		path += indexes[i];

		if (FileSystem::exists(path)) {
			indexPath = path;
			indexName = indexes[i];
			return true;
		}
	}

	return false;
}

bool FileSystem::writeFile(const std::string& path, const std::string& content) {
	if (!isWritable(path)) {
		Logger::info(std::string(" Path is not writable: ") + path);
		return false;
	}

	std::ofstream file(path.c_str(), std::ios::binary);
	if (!file) {
		Logger::error(std::string(" Failed to open file for write: ") + path);
		return false;
	}

	typedef TResourceGuard<std::ofstream&, ResourceDeleters::closeOutFileStream> ofstreamGuard;
	ofstreamGuard ofstream_guard(file);

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

/* ************************************************************************** */
