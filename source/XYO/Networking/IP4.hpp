// Networking
// Copyright (c) 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORING_IP4_HPP
#define XYO_NETWORING_IP4_HPP

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

namespace XYO::Networking {
	class IP4 : public Object {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(IP4);

		public:
			uint8_t ip[4];

			XYO_NETWORKING_EXPORT IP4();
			XYO_NETWORKING_EXPORT uint32_t toU32();
			XYO_NETWORKING_EXPORT void fromU32(uint32_t ip_);
			XYO_NETWORKING_EXPORT void copy(IP4 &ip_);
			XYO_NETWORKING_EXPORT bool isInTheSameNetwork(IP4 &ip_, IP4 &mask_);
	};
};

#endif
