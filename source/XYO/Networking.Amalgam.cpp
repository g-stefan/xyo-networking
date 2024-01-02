// Networking
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/Networking.hpp>
#include <XYO/Networking/Copyright.cpp>
#include <XYO/Networking/License.cpp>
#include <XYO/Networking/Version.cpp>

// -

#include <XYO/Networking/IP4.cpp>
#include <XYO/Networking/IP6.cpp>
#include <XYO/Networking/IPAddress4.cpp>
#include <XYO/Networking/IPAddress6.cpp>
#include <XYO/Networking/Network-OS-Linux.cpp>
#include <XYO/Networking/Network-OS-Windows.cpp>
#include <XYO/Networking/Socket-OS-Linux.cpp>
#include <XYO/Networking/Socket-OS-Windows.cpp>
#include <XYO/Networking/URL.cpp>

#ifdef XYO_OS_WINDOWS
#	pragma comment(lib, "ws2_32.lib")
#endif
