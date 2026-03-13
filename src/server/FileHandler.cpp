#include "../../include/server/FileHandler.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstring>
#include <sstream>
#include <vector>

#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/Logger.hpp"

// bool FileHandler::fileExists(const std::string& path) {
// 	struct stat st;
// 	return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
// }

// std::string FileHandler::readFile(const std::string& path) {
// 	std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
// 	if (!file) return "";

// 	std::ostringstream ss;
// 	ss << file.rdbuf();
// 	return ss.str();
// }

std::string FileHandler::getMimeType(const std::string& path) {
	if (path.find(".html") != std::string::npos) return "text/html";
	if (path.find(".htm") != std::string::npos) return "text/html";
	if (path.find(".css") != std::string::npos) return "text/css";
	if (path.find(".js") != std::string::npos) return "application/javascript";
	if (path.find(".png") != std::string::npos) return "image/png";
	if (path.find(".svg") != std::string::npos) return "image/svg+xml";
	if (path.find(".jpg") != std::string::npos) return "image/jpeg";

	return "text/plain";
}

std::string FileHandler::generateAutoIndex(const std::string& path, const std::string& uri) {
	std::string html = "<html><body><h1>Index of " + uri + "</h1><ul>";
	std::vector<std::string> files = FileSystem::listDirectory(path);

	for (size_t i = 0; i < files.size(); ++i) {
		if (!uri.empty()) {
			html += "<li><a href=\"/" + uri + '/' + files.at(i) + "\">" + files.at(i) + "</a></li>";
		} else {
			html += "<li><a href=\"" + uri + '/' + files.at(i) + "\">" + files.at(i) + "</a></li>";
		}
	}

	html += "</ul></body></html>";

	Logger::debug(" Autoindex html response: \n" + html + '\n');

	return html;
}

// bool FileHandler::isDir(const std::string& path) {
// 	struct stat st;
// 	if (stat(path.c_str(), &st) == 0) return S_ISDIR(st.st_mode);
// 	return false;
// }

// std::string FileHandler::normalizePath(const std::string& path, const std::string& location_path) {
// 	std::vector<std::string> parts;
// 	std::stringstream ss(path);
// 	std::string item;

// 	while (std::getline(ss, item, '/')) {
// 		if (item == "" || item == ".") {
// 			continue;
// 		}

// 		if (item == "..") {
// 			if (!parts.empty()) {
// 				parts.pop_back();
// 			}
// 		} else {
// 			parts.push_back(item);
// 		}
// 	}

// 	std::string result;
// 	for (size_t i = 0; i < parts.size(); ++i) {
// 		result += parts[i];
// 		if (i + 1 < parts.size()) {
// 			result += "/";
// 		}
// 	}

// 	Logger::debug(" Normalize result: " + result);
// 	Logger::debug(" Normalize location_path: " + location_path);

// 	if (!location_path.empty() && result.compare(0, location_path.length() - 1, location_path, 1, location_path.length()) == 0) {
// 		result = result.substr(location_path.length() - 1);
// 		if (result.empty() || result[0] != '/') {
// 			result = "/" + result;
// 		}
// 	}

// 	Logger::debug(" Normalized path: " + result);

// 	return result;
// }

// bool FileHandler::deleteFile(const std::string& path) {
// 	if (!fileExists(path)) {
// 		return false;
// 	}
// 	if (isDir(path)) {
// 		return false;
// 	}
// 	return std::remove(path.c_str()) == 0;
// }

// bool FileHandler::writeFile(const std::string& path, const std::string& content) {
// 	std::ofstream file(path.c_str(), std::ios::binary);
// 	if (!file.is_open()) {
// 		Logger::error(std::string(" Failed to open file for write: ") + path + " (" + strerror(errno) + ")");
// 		return false;
// 	}
// 	file.write(content.c_str(), content.size());
// 	file.close();
// 	return true;
// }
