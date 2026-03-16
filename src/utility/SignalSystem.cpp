/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalSystem.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dgibrat <dgibrat@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 20:20:57 by dgibrat           #+#    #+#             */
/*   Updated: 2026/03/16 09:43:42 by dgibrat          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/SignalSystem.hpp"

volatile sig_atomic_t SignalSystem::running = 1;

void SignalSystem::setupSignalSystem() {
	signal(SIGPIPE, SIG_IGN);

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
