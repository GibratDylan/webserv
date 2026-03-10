/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 12:49:46 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/10 13:37:42 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>

class FileSystem {
   public:
	/* ----------------- METHODS --------------------n*/

	static bool exists(const std::string& path);
	static bool isFile(const std::string& path);
	static bool isDirectory(const std::string& path);
	static bool isReadable(const std::string& path);
	static bool isWritable(const std::string& path);

	static std::string readFile(const std::string& path);

	static bool writeFile(const std::string& path, const std::string& content);

	static bool deleteFile(const std::string& path);

   private:
	FileSystem();
};

#endif	// FILESYSTEM_HPP !
