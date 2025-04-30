// Networking
// Copyright (c) 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_VERSION_HPP
#define XYO_NETWORKING_VERSION_HPP

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

namespace XYO::Networking::Version {

	XYO_NETWORKING_EXPORT const char *version();
	XYO_NETWORKING_EXPORT const char *build();
	XYO_NETWORKING_EXPORT const char *versionWithBuild();
	XYO_NETWORKING_EXPORT const char *datetime();

};

#endif
