/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PathUtils.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/13 10:58:04 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 12:36:14 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PATHUTILS_HPP
#define PATHUTILS_HPP

#include <string>

class PathUtils {
   public:
	static std::string normalize(const std::string& path);
	static std::string resolve(const std::string& base,
							   const std::string& relative);
	static std::string join(const std::string& base,
							const std::string& relative);
	static std::string getExtension(const std::string& path);
	static std::string normalizeForLocation(const std::string& path,
											const std::string& location_path);

   private:
	PathUtils();
};

#endif	// PATHUTILS_HPP !
