// Networking
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORING_IP6_HPP
#define XYO_NETWORING_IP6_HPP

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

namespace XYO::Networking {		

		class IP6 : public Object {
				XYO_DISALLOW_COPY_ASSIGN_MOVE(IP6);

			public:
				uint8_t ip[16];

				XYO_NETWORKING_EXPORT IP6();
				XYO_NETWORKING_EXPORT void copy(IP6 &ip_);
				XYO_NETWORKING_EXPORT bool isInTheSameNetwork(IP6 &ip_, IP6 &mask_);
		};
	
};

#endif
