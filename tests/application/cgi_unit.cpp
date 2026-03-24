#include <iostream>
#include <string>

#include "../../include/cgi/CgiResponseParser.hpp"

static int fail(const std::string& msg) {
	std::cerr << "[FAIL] " << msg << std::endl;
	return 1;
}

static int expect(bool condition, const std::string& msg) {
	if (!condition) {
		return fail(msg);
	}
	return 0;
}

static int testParseBodyOnly() {
	CgiResponseParser parser;
	CgiResponseParser::ParsedResponse resp = parser.parse("hello");

	if (expect(resp.code == 200, "body-only response should be 200")) {
		return 1;
	}
	if (expect(resp.type == "text/html",
			   "default content-type should be text/html")) {
		return 1;
	}
	if (expect(resp.body == "hello", "body-only response body should match")) {
		return 1;
	}
	return 0;
}

static int testParseWithHeaders() {
	CgiResponseParser parser;
	const std::string output =
		"Status: 201 Created\r\n"
		"Content-Type: text/plain\r\n"
		"X-Test: ok\r\n"
		"\r\n"
		"payload";
	CgiResponseParser::ParsedResponse resp = parser.parse(output);

	if (expect(resp.code == 201, "status header should set code")) {
		return 1;
	}
	if (expect(resp.type == "text/plain",
			   "content-type header should set type")) {
		return 1;
	}
	if (expect(resp.body == "payload", "response body should match")) {
		return 1;
	}
	if (expect(resp.headers.count("X-Test") == 1,
			   "custom header should be preserved")) {
		return 1;
	}
	return 0;
}

static int testParseEmptyOutput() {
	CgiResponseParser parser;
	CgiResponseParser::ParsedResponse resp = parser.parse("");

	if (expect(resp.code == 500, "empty output should map to 500")) {
		return 1;
	}
	if (expect(resp.body == "CGI script produced no output",
			   "empty output message should be explicit")) {
		return 1;
	}
	return 0;
}

int main() {
	if (testParseBodyOnly()) {
		return 1;
	}
	if (testParseWithHeaders()) {
		return 1;
	}
	if (testParseEmptyOutput()) {
		return 1;
	}
	std::cout << "[OK] cgi parser unit tests" << std::endl;
	return 0;
}
