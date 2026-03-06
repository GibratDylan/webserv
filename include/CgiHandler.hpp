/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 14:27:54 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/06 17:10:39 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <map>
#include <string>
#include <vector>

class Config;
class HttpResponse;

class CgiHandler {
   private:
	CgiHandler& operator=(const CgiHandler& rhs);
	CgiHandler(const CgiHandler& src);
	CgiHandler();
	static std::vector<std::string> createEnv(
		const std::string& query, const std::string& method,
		const std::string& body,
		const std::map<std::string, std::string>& headers,
		const std::string& path, const std::string& root_path, Config* config);

   public:
	CgiHandler(const std::string& path, const std::string& query,
			   const std::string& method, const std::string& body,
			   const std::map<std::string, std::string>& headers,
			   Config* config);

	~CgiHandler();

	void run();

   private:
	int _fdFromCgi[2];
	int _fdToCgi[2];
	std::vector<std::string> _env;
	std::vector<std::string> _argv;
	std::string _method;

   public:
	int code;
	std::string type;
	std::string body;
};

#endif
