/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IOMultiplexer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/14 12:36:02 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/16 09:40:46 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IOMULTIPLEXER_HPP
#define IOMULTIPLEXER_HPP

#include <poll.h>

#include <cstddef>
#include <vector>

class IOMultiplexer {
   public:
	IOMultiplexer();
	~IOMultiplexer();

	void addFd(int fd, short events);
	void modifyFd(int fd, short events);
	void removeFd(int fd);

	int poll(int timeoutMs);

	size_t size() const;
	int getFd(size_t index) const;
	short getRevents(size_t index) const;

   private:
	IOMultiplexer(const IOMultiplexer&);
	IOMultiplexer& operator=(const IOMultiplexer&);

	std::vector<pollfd> _pollFds;
};

#endif
