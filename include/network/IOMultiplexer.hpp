/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOMultiplexer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:36:02 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/14 13:07:39 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IOMULTIPLEXER_HPP
#define IOMULTIPLEXER_HPP

#include <poll.h>
#include <unistd.h>

#include <list>

class IOMultiplexer {
   public:
	enum EventType { READ = 0x01, WRITE = 0x02, ERROR = 0x04 };

   private:
	std::list<pollfd> _pollFds;

   public:
	IOMultiplexer();
	~IOMultiplexer();

	void addFd(int fd, short events);

	void modifyFd(int fd, short events);

	void removeFd(int fd);

	short getRevents(int fd) const;
};

#endif	// IOMULTIPLEXER_HPP !
