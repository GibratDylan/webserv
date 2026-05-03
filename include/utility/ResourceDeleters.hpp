/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResourceDeleters.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 16:17:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/14 12:09:33 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESOURCEDELETERS_HPP
#define RESOURCEDELETERS_HPP

#include <dirent.h>
#include <unistd.h>

#include <cstring>
#include <string>

#include "Logger.hpp"

namespace ResourceDeleters {

inline void closeDirPointer(DIR* dir) {
	if (dir != NULL && closedir(dir) == -1) {
		Logger::error(std::string(" Failed to close directory"));
	}
}

inline void closeFd(int fd) {
	if (fd != -1 && close(fd) == -1) {
		Logger::error(std::string(" Failed to close fd: "));
	}
}

}  // namespace ResourceDeleters

#endif	// RESOURCEDELETERS_HPP
