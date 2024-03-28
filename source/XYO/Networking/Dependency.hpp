// Networking
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#define XYO_NETWORKING_DEPENDENCY_HPP

#ifndef XYO_SYSTEM_HPP
#	include <XYO/System.hpp>
#endif

// -- Export

#ifdef XYO_NETWORKING_INTERNAL
#	define XYO_NETWORKING_EXPORT XYO_PLATFORM_LIBRARY_EXPORT
#else
#	define XYO_NETWORKING_EXPORT XYO_PLATFORM_LIBRARY_IMPORT
#endif

// --

namespace XYO::Networking {
	using namespace XYO::ManagedMemory;
	using namespace XYO::DataStructures;
	using namespace XYO::Encoding;
	using namespace XYO::Multithreading;
	using namespace XYO::System;
};

#endif
