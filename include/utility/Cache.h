#pragma once

#include <sys/types.h>

#include <map>
#include <string>

class HttpRequest;
class ServerConfig;

class GetResponseCache {
   private:
	struct CachedResponse {
		std::string payload;
		time_t expiresAt;
	};

	std::map<std::string, CachedResponse> _cache;
	time_t _ttlSeconds;

   public:
	GetResponseCache(time_t ttlSeconds);

	std::string buildKey(const HttpRequest& request, const ServerConfig& serverConfig) const;
	bool get(const std::string& key, std::string& payload);
	void put(const std::string& key, const std::string& payload);
};
