// Networking
// Copyright (c) 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking/IPAddress6.hpp>

namespace XYO::Networking {

	IPAddress6::IPAddress6() {
		port = 0;
	};

	void IPAddress6::copy(IPAddress6 &addr) {
		ip.copy(addr.ip);
		port = addr.port;
	};

};
