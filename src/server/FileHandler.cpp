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
