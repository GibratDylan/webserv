/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MimeTypeResolver.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 12:36:14 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MIMETYPERESOLVER_HPP
#define MIMETYPERESOLVER_HPP

#include <string>

class MimeTypeResolver {
   public:
	static std::string resolve(const std::string& path);

   private:
	MimeTypeResolver();
};

#endif	// MIMETYPERESOLVER_HPP
