// Networking
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_COPYRIGHT_HPP
#define XYO_NETWORKING_COPYRIGHT_HPP

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

namespace XYO::Networking::Copyright {
	XYO_NETWORKING_EXPORT std::string copyright();
	XYO_NETWORKING_EXPORT std::string publisher();
	XYO_NETWORKING_EXPORT std::string company();
	XYO_NETWORKING_EXPORT std::string contact();
};

#endif
