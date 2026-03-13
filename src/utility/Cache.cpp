#include "../../include/utility/Cache.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>

#include "../../include/HttpRequest.h"
#include "../../include/config/ServerConfig.hpp"
#include "../../include/utils.h"

GetResponseCache::GetResponseCache(time_t ttlSeconds) : _ttlSeconds(ttlSeconds) {}

std::string GetResponseCache::buildKey(const HttpRequest& request, const ServerConfig& serverConfig) const {
	std::string key = toString(serverConfig.port) + "|" + request.path;
	if (!request.query.empty()) key += "?" + request.query;
	return key;
}

bool GetResponseCache::get(const std::string& key, std::string& payload) {
	std::map<std::string, CachedResponse>::iterator it = _cache.find(key);
	if (it == _cache.end()) return false;

	time_t now = std::time(NULL);
	if (it->second.expiresAt <= now) {
		_cache.erase(it);
		return false;
	}

	payload = it->second.payload;
	return true;
}

void GetResponseCache::put(const std::string& key, const std::string& payload) {
	CachedResponse entry;
	entry.payload = payload;
	entry.expiresAt = std::time(NULL) + _ttlSeconds;
	_cache[key] = entry;
}
