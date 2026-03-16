/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 11:31:39 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/16 12:44:10 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <list>
#include <map>
#include <string>
#include <vector>

class Config {
   public:
	Config();
	Config(const Config& src);
	Config(const std::string& localDirective, const Config& serverConfig);

	virtual ~Config();

	Config& operator=(const Config& rhs);

   protected:
	void handleListen(const std::list<std::string>& words);
	void handleRoot(const std::list<std::string>& words);
	void handleIndex(const std::list<std::string>& words);
	void handleErrorPage(const std::list<std::string>& words);
	void handleAutoIndex(const std::list<std::string>& words);
	void handleClientMaxBodySize(const std::list<std::string>& words);
	void handleLargeClientHeaderBuffers(const std::list<std::string>& words);
	void handleClientHeaderBufferSize(const std::list<std::string>& words);
	void handleUploadStore(const std::list<std::string>& words);
	void handleMethods(const std::list<std::string>& words);
	void handleRedirection(const std::list<std::string>& words);
	void handleCGI(const std::list<std::string>& words);
	void handleMaxConnections(const std::list<std::string>& words);
	void handleSessionTimeout(const std::list<std::string>& words);

   private:
	void parseLocalDirective(const std::string& localDirective);

   public:
	std::string host;
	int port;
	std::string root;
	std::string location_path;
	std::vector<std::string> index;
	bool autoindex;
	size_t client_max_body_size;
	std::map<int, std::string> error_pages;
	std::vector<std::string> methods;
	std::pair<int, std::string> redirection;
	size_t large_client_header_buffers;
	size_t client_header_buffer_size;
	std::string upload_store;
	std::map<std::string, std::string> cgi_handlers;
	size_t max_connections;
	size_t session_timeout;
	// bool isFile;
	bool root_explicitly_set;
};

#endif
