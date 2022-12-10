// Networking
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

#ifdef XYO_OS_WINDOWS

#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <winsock2.h>

#	include <XYO/Networking/Network.hpp>

namespace XYO::Networking::Network {

	class Network_ {
		public:
			bool isValid;

			Network_();
			~Network_();
	};

	Network_::Network_() {
		isValid = false;
		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
		if (WSAStartup(wVersionRequested, &wsaData) == 0) {
			isValid = true;
		};
	};

	Network_::~Network_() {
		if (isValid) {
			WSACleanup();
		};
	};

	bool isValid() {
		return (TSingletonProcess<Network_>::getValue())->isValid;
	};

};

#endif
