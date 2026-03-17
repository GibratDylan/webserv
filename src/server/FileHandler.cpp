#include "../../include/server/FileHandler.hpp"

#include <sstream>
#include <vector>

#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/Logger.hpp"

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

std::string FileHandler::normalizePath(const std::string& path, const std::string& location_path) {
	std::vector<std::string> parts;
	std::stringstream ss(path);
	std::string item;

	int last_slash = !path.empty() && path[path.size() - 1] == '/';
	while (std::getline(ss, item, '/')) {
		if (item == "" || item == ".") {
			continue;
		}

		if (item == "..") {
			if (!parts.empty()) {
				parts.pop_back();
			}
		} else {
			parts.push_back(item);
		}
	}

	std::string result;
	for (size_t i = 0; i < parts.size(); ++i) {
		result += parts[i];
		if (i + 1 < parts.size()) {
			result += "/";
		}
	}

	std::string normalizedLocation = location_path;
	while (!normalizedLocation.empty() && normalizedLocation[0] == '/') {
		normalizedLocation.erase(0, 1);
	}

	// while (!normalizedLocation.empty() && normalizedLocation[normalizedLocation.size() - 1] == '/') {
	// 	normalizedLocation.erase(normalizedLocation.size() - 1);
	// }

	bool startsWithLocation = !normalizedLocation.empty() && result.compare(0, normalizedLocation.size(), normalizedLocation) == 0 &&
							  (result.size() == normalizedLocation.size() || result[normalizedLocation.size()] == '/');

	if (startsWithLocation) {
		result = result.substr(normalizedLocation.size());
		if (result.empty() || result[0] != '/') {
			result = "/" + result;
		}
	}

	if (last_slash) result = result + '/';

	Logger::debug(" Normalized path: " + result);

	return result;
}
