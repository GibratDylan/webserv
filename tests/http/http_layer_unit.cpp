#include <iostream>
#include <string>

#include "../../include/config/Config.hpp"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/http/HttpParser.hpp"

static int fail(const std::string& msg) {
	std::cerr << "[FAIL] " << msg << std::endl;
	return 1;
}

static int expect(bool condition, const std::string& msg) {
	if (!condition) return fail(msg);
	return 0;
}

int main() {
	try {
		Config global;
		ServerConfig server(
			"listen 8080; root ./www; allow_methods GET POST DELETE;", global);

		HttpParser parser(server);
		HttpRequest req;

		{
			std::string raw =
				"GET /index.html HTTP/1.1\r\n"
				"Host: example\r\n"
				"Connection: close\r\n\r\n";
			ParseStatus st = parser.parse(raw, req);
			if (expect(st == PARSE_OK, "GET should parse")) return 1;
			if (expect(req.method == "GET", "method parsed")) return 1;
			if (expect(req.path == "/index.html", "path parsed")) return 1;
			if (expect(req.version == "HTTP/1.1", "version parsed")) return 1;
			if (expect(req.getHeader("Host") == "example", "header parsed"))
				return 1;
			parser.reset();
		}

		{
			std::string raw =
				"POST /upload HTTP/1.1\r\n"
				"Host: example\r\n\r\n";
			ParseStatus st = parser.parse(raw, req);
			if (expect(st == LENGTH_REQUIRED,
					   "POST without framing should be 411"))
				return 1;
			parser.reset();
		}

		{
			std::string raw =
				"POST /upload HTTP/1.1\r\n"
				"Host: example\r\n"
				"Content-Length: 10\r\n\r\n"
				"12345";
			ParseStatus st = parser.parse(raw, req);
			if (expect(st == PARSE_INCOMPLETE,
					   "incomplete Content-Length body should be incomplete"))
				return 1;
			parser.reset();
		}

		{
			std::string raw =
				"POST /upload HTTP/1.1\r\n"
				"Host: example\r\n"
				"Transfer-Encoding: chunked\r\n\r\n"
				"4\r\nWiki\r\n"
				"5\r\npedia\r\n"
				"0\r\n\r\n";
			ParseStatus st = parser.parse(raw, req);
			if (expect(st == PARSE_OK, "chunked should parse")) return 1;
			if (expect(req.body == "Wikipedia", "chunked decoded")) return 1;
			parser.reset();
		}

		{
			std::string longPath(8200, 'a');
			std::string raw =
				"GET /" + longPath + " HTTP/1.1\r\nHost: x\r\n\r\n";
			ParseStatus st = parser.parse(raw, req);
			if (expect(st == URI_TOO_LONG, "very long URI should be 414"))
				return 1;
			parser.reset();
		}

		std::cout << "[OK] http unit tests" << std::endl;
		return 0;
	} catch (const std::exception& e) {
		return fail(std::string("exception: ") + e.what());
	}
}
