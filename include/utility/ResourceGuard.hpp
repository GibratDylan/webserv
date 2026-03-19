/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResourceGuard.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 00:00:00 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/31 12:36:14 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESOURCEGUARD_HPP
#define RESOURCEGUARD_HPP

template <typename T, void (*Deleter)(T)>
class ResourceGuard {
   private:
	T _resource;
	bool _owned;

   public:
	explicit ResourceGuard(T resource) : _resource(resource), _owned(true) {}

	~ResourceGuard() {
		if (_owned && _resource) {
			Deleter(_resource);
		}
	}

	T get() const { return _resource; }

	T release() {
		_owned = false;
		return _resource;
	}

	void reset() {
		if (_owned && _resource) {
			Deleter(_resource);
			_owned = false;
		}
	}

   private:
	ResourceGuard(const ResourceGuard&);
	ResourceGuard& operator=(const ResourceGuard&);
};

#endif
