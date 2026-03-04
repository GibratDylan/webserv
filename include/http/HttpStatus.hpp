/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpStatus.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 14:36:21 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/04 14:54:23 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSTATUS_HPP
#define HTTPSTATUS_HPP

#include <map>
#include <string>

class HttpStatus {
   public:
	static std::string getMessage(int code);

	static bool isValid(int code);

	static std::map<int, std::string> initMessages();

	static bool isSuccess(int code);
	static bool isRedirection(int code);
	static bool isClientError(int code);
	static bool isServerError(int code);

   private:
	static std::map<int, std::string> _messages;
};

#endif
