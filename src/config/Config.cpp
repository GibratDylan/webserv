/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 11:31:38 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/12 18:55:12 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/config/Config.hpp"

Config::Config()
	: host("127.0.0.1"),
	  port(8080),
	  root("./www"),
	  autoindex(false),
	  client_max_body_size(10000000),
	  large_client_header_buffers(8192),
	  client_header_buffer_size(8192),
	  max_connections(512),
	  session_timeout(3600),
	  root_explicitly_set(false) {
	methods.push_back("GET");
	methods.push_back("POST");
	redirection.first = 0;
}

Config::Config(const Config& src)
	: host(src.host),
	  port(src.port),
	  root(src.root),
	  location_path(src.location_path),
	  index(src.index),
	  autoindex(src.autoindex),
	  client_max_body_size(src.client_max_body_size),
	  error_pages(src.error_pages),
	  methods(src.methods),
	  redirection(src.redirection),
	  large_client_header_buffers(src.large_client_header_buffers),
	  client_header_buffer_size(src.client_header_buffer_size),
	  upload_store(src.upload_store),
	  cgi_handlers(src.cgi_handlers),
	  max_connections(src.max_connections),
	  session_timeout(src.session_timeout),
	  root_explicitly_set(src.root_explicitly_set) {}

Config::~Config() {}

Config& Config::operator=(const Config& rhs) {
	if (this != &rhs) {
		host = rhs.host;
		port = rhs.port;
		root = rhs.root;
		location_path = rhs.location_path;
		index = rhs.index;
		autoindex = rhs.autoindex;
		client_max_body_size = rhs.client_max_body_size;
		error_pages = rhs.error_pages;
		methods = rhs.methods;
		redirection = rhs.redirection;
		large_client_header_buffers = rhs.large_client_header_buffers;
		client_header_buffer_size = rhs.client_header_buffer_size;
		upload_store = rhs.upload_store;
		cgi_handlers = rhs.cgi_handlers;
		max_connections = rhs.max_connections;
		session_timeout = rhs.session_timeout;
		root_explicitly_set = rhs.root_explicitly_set;
	}
	return *this;
}
