// Networking
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_URL_HPP
#define XYO_NETWORKING_URL_HPP

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

namespace XYO::Networking::URL {			

			XYO_NETWORKING_EXPORT String decodeComponent(const String &value);
			XYO_NETWORKING_EXPORT String encodeComponent(const String &value);
			XYO_NETWORKING_EXPORT bool getSchemeName(const String &url, String &out);
			XYO_NETWORKING_EXPORT bool getHostNameAndPort(const String &url, String &out);
			XYO_NETWORKING_EXPORT bool getUsernameAndPassword(const String &url, String &out);
			XYO_NETWORKING_EXPORT bool getPathAndFileName(const String &url, String &out);
			XYO_NETWORKING_EXPORT bool getQuery(const String &url, String &out);
		
};

#endif
