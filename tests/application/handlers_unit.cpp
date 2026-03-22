#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../../include/application/DeleteHandler.hpp"
#include "../../include/application/DirectoryListingHandler.hpp"
#include "../../include/application/StaticFileHandler.hpp"
#include "../../include/application/UploadHandler.hpp"
#include "../../include/config/Config.hpp"
#include "../../include/server/HttpRequest.hpp"
#include "../../include/server/utils.hpp"
#include "../../include/utility/FileSystem.hpp"
#include "../../include/utility/PathUtils.hpp"

static int fail(const std::string& msg) {
	std::cerr << "[FAIL] " << msg << std::endl;
	return 1;
}

static int expect(bool condition, const std::string& msg) {
	if (!condition) return fail(msg);
	return 0;
}

static std::string makeTempDir() {
	char tpl[] = "/tmp/webserv_handlers_XXXXXX";
	char* out = ::mkdtemp(tpl);
	if (!out) {
		throw std::runtime_error(std::string("mkdtemp failed: ") +
								 std::strerror(errno));
	}
	return std::string(out);
}

static void mustMkdir(const std::string& path) {
	if (::mkdir(path.c_str(), 0700) != 0) {
		throw std::runtime_error(std::string("mkdir failed: ") + path + " : " +
								 std::strerror(errno));
	}
}

static void mustRmdir(const std::string& path) {
	if (::rmdir(path.c_str()) != 0) {
		throw std::runtime_error(std::string("rmdir failed: ") + path + " : " +
								 std::strerror(errno));
	}
}

static void mustUnlink(const std::string& path) {
	if (::unlink(path.c_str()) != 0) {
		throw std::runtime_error(std::string("unlink failed: ") + path + " : " +
								 std::strerror(errno));
	}
}

static Config makeBaseConfig(const std::string& root,
							 const std::string& uploadStore) {
	Config cfg;
	cfg.root = root;
	cfg.location_path = "";
	cfg.autoindex = false;
	cfg.index.clear();
	cfg.index.push_back("index.html");
	cfg.upload_store = uploadStore;
	cfg.client_max_body_size = 1024 * 1024;
	cfg.methods.clear();
	cfg.methods.push_back("GET");
	cfg.methods.push_back("POST");
	cfg.methods.push_back("DELETE");
	return cfg;
}

static int testStaticFileServesFile() {
	std::string root = makeTempDir();
	std::string uploads = root + "/uploads";
	mustMkdir(uploads);
	Config cfg = makeBaseConfig(root, uploads);

	std::string filePath = root + "/hello.txt";
	if (!FileSystem::writeFile(filePath, "abc")) {
		return fail("writeFile failed");
	}

	HttpRequest req;
	req.method = "GET";
	req.path = "/hello.txt";

	StaticFileHandler h;
	if (expect(h.canHandle(req, cfg), "static handler should accept GET file"))
		return 1;

	HttpResponse res = h.handle(req, cfg);
	if (expect(res.statusCode == 200, "GET file should be 200")) return 1;
	if (expect(res.body == "abc", "GET file body should match")) return 1;

	mustUnlink(filePath);
	mustRmdir(uploads);
	mustRmdir(root);
	return 0;
}

static int testDirectoryListingAutoindex() {
	std::string root = makeTempDir();
	std::string uploads = root + "/uploads";
	mustMkdir(uploads);
	Config cfg = makeBaseConfig(root, uploads);
	cfg.autoindex = true;

	std::string dirPath = root + "/dir";
	mustMkdir(dirPath);

	HttpRequest req;
	req.method = "GET";
	req.path = "/dir";

	DirectoryListingHandler h;
	if (expect(h.canHandle(req, cfg),
			   "dir listing handler should accept autoindex dir"))
		return 1;

	HttpResponse res = h.handle(req, cfg);
	if (expect(res.statusCode == 200, "autoindex should be 200")) return 1;
	if (expect(res.headers["Content-Type"] == "text/html",
			   "autoindex should be text/html"))
		return 1;
	if (expect(res.body.find("Index of dir") != std::string::npos,
			   "autoindex body should contain title"))
		return 1;

	mustRmdir(dirPath);
	mustRmdir(uploads);
	mustRmdir(root);
	return 0;
}

static int testStaticFileDirectoryForbiddenWhenNoAutoindex() {
	std::string root = makeTempDir();
	std::string uploads = root + "/uploads";
	mustMkdir(uploads);
	Config cfg = makeBaseConfig(root, uploads);
	cfg.autoindex = false;

	std::string dirPath = root + "/dir";
	mustMkdir(dirPath);

	HttpRequest req;
	req.method = "GET";
	req.path = "/dir";

	StaticFileHandler h;
	if (expect(h.canHandle(req, cfg),
			   "static handler should accept dir without autoindex"))
		return 1;

	HttpResponse res = h.handle(req, cfg);
	if (expect(res.statusCode == 403,
			   "directory without index/autoindex should be 403"))
		return 1;

	mustRmdir(dirPath);
	mustRmdir(uploads);
	mustRmdir(root);
	return 0;
}

static std::string computeUploadPath(const Config& cfg,
									 const std::string& uriPath) {
	std::string safePath =
		PathUtils::normalizeForLocation(uriPath, cfg.location_path);
	std::string uploadPath = addPath(cfg.upload_store, safePath);
	if (FileSystem::isDirectory(uploadPath))
		uploadPath = addPath(uploadPath, uriPath);
	return uploadPath;
}

static int testUploadThenDelete() {
	std::string root = makeTempDir();
	std::string uploads = root + "/uploads";
	mustMkdir(uploads);
	Config cfg = makeBaseConfig(root, uploads);

	HttpRequest post;
	post.method = "POST";
	post.path = "/file.txt";
	post.body = "payload";

	UploadHandler uploader;
	if (expect(uploader.canHandle(post, cfg), "upload handler accepts POST"))
		return 1;

	HttpResponse postRes = uploader.handle(post, cfg);
	if (expect(postRes.statusCode == 201, "upload should be 201")) return 1;

	std::string storedPath = computeUploadPath(cfg, post.path);
	if (expect(FileSystem::exists(storedPath),
			   "uploaded file should exist in upload_store"))
		return 1;

	HttpRequest del;
	del.method = "DELETE";
	del.path = "/file.txt";

	DeleteHandler deleter;
	if (expect(deleter.canHandle(del, cfg), "delete handler accepts DELETE"))
		return 1;

	HttpResponse delRes = deleter.handle(del, cfg);
	if (expect(delRes.statusCode == 204, "delete should be 204")) return 1;
	if (expect(!FileSystem::exists(storedPath), "deleted file should be gone"))
		return 1;

	mustRmdir(uploads);
	mustRmdir(root);
	return 0;
}

int main() {
	try {
		if (testStaticFileServesFile()) return 1;
		if (testDirectoryListingAutoindex()) return 1;
		if (testStaticFileDirectoryForbiddenWhenNoAutoindex()) return 1;
		if (testUploadThenDelete()) return 1;

		std::cout << "[OK] application handler unit tests" << std::endl;
		return 0;
	} catch (const std::exception& e) {
		return fail(std::string("exception: ") + e.what());
	}
}
