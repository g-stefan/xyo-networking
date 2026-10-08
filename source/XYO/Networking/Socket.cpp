// Networking
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking/Socket.hpp>
#include <XYO/Networking/URL.hpp>

namespace XYO::Networking {

	// Accepts: ip4:port, host:port, [ip6]:port, ip6, scheme://host[:port]/...
	bool socketParseAddress_(const String &adr_, String &host, uint16_t &port) {
		String schemeName;
		String hostAndPort;
		String portText;
		size_t index;
		size_t indexFromEnd;
		uint32_t value;

		if (!URL::getSchemeName(adr_, schemeName)) {
			schemeName = "none";
		};
		if (!URL::getHostNameAndPort(adr_, hostAndPort)) {
			hostAndPort = adr_;
		};

		if (hostAndPort[0] == '[') {
			if (!hostAndPort.indexOf("]", 0, index)) {
				return false;
			};
			host = hostAndPort.substring(1, index - 1);
			if (index + 1 < hostAndPort.length()) {
				if (hostAndPort[index + 1] != ':') {
					return false;
				};
				portText = hostAndPort.substring(index + 2);
			};
		} else if (hostAndPort.indexOf(":", 0, index)) {
			indexFromEnd = index;
			StringCore::indexOfFromEnd(hostAndPort, ":", indexFromEnd);
			if (index == indexFromEnd) {
				host = hostAndPort.substring(0, index);
				portText = hostAndPort.substring(index + 1);
			} else {
				// IPv6 without port
				host = hostAndPort;
			};
		} else {
			host = hostAndPort;
		};

		if (host.length() == 0) {
			return false;
		};

		if (portText.length() == 0) {
			if (schemeName.matchASCII("http")) {
				port = 80;
			} else if (schemeName.matchASCII("https")) {
				port = 443;
			} else if (schemeName.matchASCII("ftp")) {
				port = 21;
			} else if (schemeName.matchASCII("ssh")) {
				port = 22;
			} else if (schemeName.matchASCII("telnet")) {
				port = 23;
			} else {
				return false;
			};
			return true;
		};

		value = 0;
		for (index = 0; index < portText.length(); ++index) {
			if ((portText[index] < '0') || (portText[index] > '9')) {
				return false;
			};
			value = value * 10 + (portText[index] - '0');
			if (value > 65535) {
				return false;
			};
		};
		port = static_cast<uint16_t>(value);
		return true;
	};

	bool Socket::isIPAddress4() {
		if (!ipAddress) {
			return false;
		};
		return (!ipAddressIs6);
	};

	bool Socket::isIPAddress6() {
		if (!ipAddress) {
			return false;
		};
		return ipAddressIs6;
	};

	bool Socket::getIPAddress4(IPAddress4 &address_) {
		if (!ipAddress) {
			return false;
		};
		if (ipAddressIs6) {
			return false;
		};
		address_.copy(*(reinterpret_cast<IPAddress4 *>(ipAddress)));
		return true;
	};

	bool Socket::getIPAddress6(IPAddress6 &address_) {
		if (!ipAddress) {
			return false;
		};
		if (!ipAddressIs6) {
			return false;
		};
		address_.copy(*(reinterpret_cast<IPAddress6 *>(ipAddress)));
		return true;
	};

};
