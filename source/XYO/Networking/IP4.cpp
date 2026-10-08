// Networking
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking/IP4.hpp>

namespace XYO::Networking {

	IP4::IP4() {
		memset(&ip[0], 0, 4);
	};

	uint32_t IP4::toU32() {
		uint32_t retV;
		memcpy(&retV, &ip[0], 4);
		return retV;
	};

	void IP4::fromU32(uint32_t ip_) {
		memcpy(&ip[0], &ip_, 4);
	};

	void IP4::copy(IP4 &ip_) {
		memcpy(&ip[0], &ip_.ip[0], 4);
	};

	bool IP4::isInTheSameNetwork(IP4 &ip_, IP4 &mask_) {
		int k;
		for (k = 0; k < 4; ++k) {
			if ((ip[k] & mask_.ip[k]) != (ip_.ip[k] & mask_.ip[k])) {
				return false;
			};
		};
		return true;
	};

};
