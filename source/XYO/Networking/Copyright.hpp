// Networking
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_COPYRIGHT_HPP
#define XYO_NETWORKING_COPYRIGHT_HPP

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

namespace XYO::Networking::Copyright {
	XYO_NETWORKING_EXPORT const char *copyright();
	XYO_NETWORKING_EXPORT const char *publisher();
	XYO_NETWORKING_EXPORT const char *company();
	XYO_NETWORKING_EXPORT const char *contact();
};

#endif
