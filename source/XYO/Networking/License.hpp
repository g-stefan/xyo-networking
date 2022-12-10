// Networking
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_LICENSE_HPP
#define XYO_NETWORKING_LICENSE_HPP

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

namespace XYO::Networking::License {

	XYO_NETWORKING_EXPORT const char *license();
	XYO_NETWORKING_EXPORT const char *shortLicense();

};

#endif
