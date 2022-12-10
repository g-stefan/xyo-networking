// Networking
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking/ip6.hpp>

namespace XYO::Networking {

	IP6::IP6() {
		memset(&ip[0], 0, 16);
	};

	void IP6::copy(IP6 &ip_) {
		memcpy(&ip[0], &ip_.ip[0], 16);
	};

	bool IP6::isInTheSameNetwork(IP6 &ip_, IP6 &mask_) {
		int k;
		for (k = 0; k < 16; ++k) {
			if ((ip[k] & mask_.ip[k]) != (ip_.ip[k] & mask_.ip[k])) {
				return false;
			};
		};
		return true;
	};

};
