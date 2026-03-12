/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalSystem.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 20:20:57 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/12 20:35:02 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/SignalSystem.hpp"

/*
** --------------------------------- METHODS ----------------------------------
*/

volatile sig_atomic_t SignalSystem::running = 1;

/* ----------------- METHODS --------------------n*/
void SignalSystem::setupSignalSystem() {
	struct sigaction sig;
	sig.sa_handler = handlerSigintSignal;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = SA_RESTART;

	sigaction(SIGINT, &sig, NULL);
}

void SignalSystem::handlerSigintSignal(int sig) {
	static_cast<void>(sig);
	running = 0;
}

/* ************************************************************************** */
