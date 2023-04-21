// Networking
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_SOCKET_HPP
#define XYO_NETWORKING_SOCKET_HPP

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

#ifndef XYO_NETWORKING_IPADRESS4_HPP
#	include <XYO/Networking/IPAddress4.hpp>
#endif

#ifndef XYO_NETWORKING_IPADRESS6_HPP
#	include <XYO/Networking/IPAddress6.hpp>
#endif

namespace XYO::Networking {

	class IPAddress_;
	class Socket_;

	class Socket : public virtual IRead,
	               public virtual IWrite {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(Socket);

		protected:
			Socket_ *this_;			
			IPAddress_ *ipAddress;
			bool ipAddressIs6;

		public:
			XYO_NETWORKING_EXPORT Socket();
			XYO_NETWORKING_EXPORT ~Socket();
			XYO_NETWORKING_EXPORT operator bool() const;

			XYO_NETWORKING_EXPORT bool openClient(IPAddress4 &adr_);
			XYO_NETWORKING_EXPORT bool openServer(IPAddress4 &adr_);
			XYO_NETWORKING_EXPORT bool openClient(IPAddress6 &adr_);
			XYO_NETWORKING_EXPORT bool openServer(IPAddress6 &adr_);
			XYO_NETWORKING_EXPORT bool listen(uint16_t queue_);
			XYO_NETWORKING_EXPORT bool accept(Socket &socket_);
			XYO_NETWORKING_EXPORT void close();

			XYO_NETWORKING_EXPORT size_t read(void *output, size_t ln);
			XYO_NETWORKING_EXPORT size_t write(const void *input, size_t ln);
			XYO_NETWORKING_EXPORT int waitToWrite(uint32_t microSeconds);
			XYO_NETWORKING_EXPORT int waitToRead(uint32_t microSeconds);

			XYO_NETWORKING_EXPORT bool openClientX(const String &adr_);
			XYO_NETWORKING_EXPORT bool openServerX(const String &adr_);

			XYO_NETWORKING_EXPORT bool isIPAddress4();
			XYO_NETWORKING_EXPORT bool isIPAddress6();
			XYO_NETWORKING_EXPORT bool getIPAddress4(IPAddress4 &);
			XYO_NETWORKING_EXPORT bool getIPAddress6(IPAddress6 &);
			
	};
};

#endif
