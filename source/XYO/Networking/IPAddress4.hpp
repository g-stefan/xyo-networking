// Networking
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_IPADDRESS4_HPP
#define XYO_NETWORKING_IPADDRESS4_HPP

#ifndef XYO_NETWORKING_IP4_HPP
#	include <XYO/Networking/IP4.hpp>
#endif

namespace XYO::Networking {

	class IPAddress4 : public Object {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(IPAddress4);

		public:
			IP4 ip;
			uint16_t port;

			XYO_NETWORKING_EXPORT IPAddress4();
			XYO_NETWORKING_EXPORT void copy(IPAddress4 &addr);
	};

};

#endif
