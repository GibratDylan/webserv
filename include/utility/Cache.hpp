/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cache.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 13:55:31 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 14:53:22 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CACHE_HPP
#define CACHE_HPP

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

	std::string buildKey(const HttpRequest& request,
						 const ServerConfig& serverConfig) const;
	bool get(const std::string& key, std::string& payload);
	void put(const std::string& key, const std::string& payload);
};

#endif
