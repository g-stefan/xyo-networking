// Networking
// Copyright (c) 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_IPADDRESS6_HPP
#define XYO_NETWORKING_IPADDRESS6_HPP

#ifndef XYO_NETWORKING_IP6_HPP
#	include <XYO/Networking/IP6.hpp>
#endif

namespace XYO::Networking {
	using namespace XYO::ManagedMemory;

	class IPAddress6 : public Object {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(IPAddress6);

		public:
			IP6 ip;
			uint16_t port;

			XYO_NETWORKING_EXPORT IPAddress6();
			XYO_NETWORKING_EXPORT void copy(IPAddress6 &addr);
	};

};

#endif
