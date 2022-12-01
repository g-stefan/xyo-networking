// Networking
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking/IPAddress4.hpp>

namespace XYO::Networking {

		IPAddress4::IPAddress4() {
			port = 0;
		};

		void IPAddress4::copy(IPAddress4 &addr) {
			ip.copy(addr.ip);
			port = addr.port;
		};
	
};
