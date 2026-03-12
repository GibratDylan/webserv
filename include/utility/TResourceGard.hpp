/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TResourceGard.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 15:50:08 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/12 21:02:08 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TRESOURCEGUARD_HPP
#define TRESOURCEGUARD_HPP

template <typename T, void (*Deleter)(T)>
class TResourceGuard {
   private:
	T _resource;
	bool _owned;

   public:
	explicit TResourceGuard(T resource) : _resource(resource), _owned(true) {}

	~TResourceGuard() {
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
	TResourceGuard(const TResourceGuard&);
	TResourceGuard& operator=(const TResourceGuard&);
};

#endif
