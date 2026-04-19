/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MimeTypeResolver.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/04/19 14:37:17 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/MimeTypeResolver.hpp"

std::string MimeTypeResolver::resolve(const std::string& path) {
	// Preserve current behavior: substring matching, not strict extension
	// parsing.
	if (path.find(".html") != std::string::npos) return "text/html";
	if (path.find(".htm") != std::string::npos) return "text/html";
	if (path.find(".css") != std::string::npos) return "text/css";
	if (path.find(".js") != std::string::npos) return "application/javascript";
	if (path.find(".png") != std::string::npos) return "image/png";
	if (path.find(".svg") != std::string::npos) return "image/svg+xml";
	if (path.find(".jpg") != std::string::npos) return "image/jpeg";

	return "text/plain";
}
