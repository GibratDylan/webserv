/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalSystem.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 20:20:57 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/12 20:33:36 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALSYSTEM_HPP
#define SIGNALSYSTEM_HPP

#include <csignal>

class SignalSystem {
   public:
	static volatile sig_atomic_t running;

   public:
	/* ----------------- METHODS --------------------n*/
	static void setupSignalSystem();
	static void handlerSigintSignal(int sig);

   private:
	SignalSystem();
};

#endif	// SIGNALSYSTEM_HPP !
