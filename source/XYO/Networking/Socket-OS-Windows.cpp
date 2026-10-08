// Networking
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

#ifdef XYO_PLATFORM_OS_WINDOWS

#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <limits.h>

#	include <XYO/Networking/Network.hpp>
#	include <XYO/Networking/Socket.hpp>
#	include <XYO/Networking/URL.hpp>

namespace XYO::Networking {

	class Socket_ {
		public:
			SOCKET value;
	};

	// Socket.cpp
	bool socketParseAddress_(const String &adr_, String &host, uint16_t &port);

	static IPAddress_ *socketNewIPAddress_(const struct sockaddr *addr, bool &ipAddressIs6) {
		if (addr->sa_family == AF_INET6) {
			const struct sockaddr_in6 *addr6 = reinterpret_cast<const struct sockaddr_in6 *>(addr);
			IPAddress6 *ipAddress6 = new IPAddress6();
			ipAddress6->port = ntohs(addr6->sin6_port);
			ipAddress6->scopeId = addr6->sin6_scope_id;
			memcpy(&ipAddress6->ip.ip[0], &addr6->sin6_addr, 16);
			ipAddressIs6 = true;
			return reinterpret_cast<IPAddress_ *>(ipAddress6);
		};

		const struct sockaddr_in *addr4 = reinterpret_cast<const struct sockaddr_in *>(addr);
		IPAddress4 *ipAddress4 = new IPAddress4();
		ipAddress4->port = ntohs(addr4->sin_port);
		memcpy(&ipAddress4->ip.ip[0], &addr4->sin_addr, 4);
		ipAddressIs6 = false;
		return reinterpret_cast<IPAddress_ *>(ipAddress4);
	};

	// Windows fd_set is an array of socket handles, not a bitmap indexed
	// by socket value, select is not limited by the number of sockets
	static int socketWait_(SOCKET value, bool toWrite, uint32_t microSeconds) {
		int ret;
		fd_set sock_set;
		struct timeval timev;

		timev.tv_sec = (microSeconds / (1000 * 1000));
		timev.tv_usec = (microSeconds % (1000 * 1000));
		FD_ZERO(&sock_set);
		FD_SET(value, &sock_set);
		if (toWrite) {
			ret = select(0, nullptr, &sock_set, nullptr, &timev);
		} else {
			ret = select(0, &sock_set, nullptr, nullptr, &timev);
		};
		if (ret == SOCKET_ERROR) {
			return -1;
		};
		if (ret == 0) {
			return 0;
		};
		return 1;
	};

	static bool socketOpen_(Socket_ *this_, IPAddress_ *&ipAddress, bool &ipAddressIs6, const struct sockaddr *addr, int addrLn, bool isServer) {
		// Socket handle must not be inherited by child processes
		this_->value = WSASocketW(addr->sa_family, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED | WSA_FLAG_NO_HANDLE_INHERIT);
		if (this_->value == INVALID_SOCKET) {
			return false;
		};

		if (isServer) {
			// SO_REUSEADDR is not set, on Windows it allows other processes to take over the port
			if (addr->sa_family == AF_INET6) {
				// Accept IPv4 connections too, same as on Linux
				DWORD option = 0;
				setsockopt(this_->value, IPPROTO_IPV6, IPV6_V6ONLY, (const char *)&option, sizeof(option));
			};
			if (bind(this_->value, addr, addrLn) == 0) {
				ipAddress = socketNewIPAddress_(addr, ipAddressIs6);
				return true;
			};
		} else {
			if (connect(this_->value, addr, addrLn) == 0) {
				ipAddress = socketNewIPAddress_(addr, ipAddressIs6);
				return true;
			};
		};

		closesocket(this_->value);
		this_->value = INVALID_SOCKET;
		return false;
	};

	static bool socketOpenX_(Socket_ *this_, IPAddress_ *&ipAddress, bool &ipAddressIs6, const String &adr_, bool isServer) {
		String host;
		uint16_t port;
		struct addrinfo hints;
		struct addrinfo *result;
		struct addrinfo *scan;

		if (!socketParseAddress_(adr_, host, port)) {
			return false;
		};

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = isServer ? AI_PASSIVE : 0;

		if (getaddrinfo(host, nullptr, &hints, &result) != 0) {
			return false;
		};
		// Use the resolved address as is, keeps IPv6 scope id
		for (scan = result; scan != nullptr; scan = scan->ai_next) {
			if (scan->ai_family == AF_INET) {
				(reinterpret_cast<struct sockaddr_in *>(scan->ai_addr))->sin_port = htons(port);
			} else if (scan->ai_family == AF_INET6) {
				(reinterpret_cast<struct sockaddr_in6 *>(scan->ai_addr))->sin6_port = htons(port);
			} else {
				continue;
			};
			if (socketOpen_(this_, ipAddress, ipAddressIs6, scan->ai_addr, static_cast<int>(scan->ai_addrlen), isServer)) {
				freeaddrinfo(result);
				return true;
			};
		};

		freeaddrinfo(result);
		return false;
	};

	static int socketAddress_(IPAddress4 &adr_, struct sockaddr_storage &addr) {
		struct sockaddr_in *addr4 = reinterpret_cast<struct sockaddr_in *>(&addr);
		memset(&addr, 0, sizeof(struct sockaddr_storage));
		addr4->sin_family = AF_INET;
		addr4->sin_port = htons(adr_.port);
		memcpy(&addr4->sin_addr, &adr_.ip.ip[0], 4);
		return sizeof(struct sockaddr_in);
	};

	static int socketAddress_(IPAddress6 &adr_, struct sockaddr_storage &addr) {
		struct sockaddr_in6 *addr6 = reinterpret_cast<struct sockaddr_in6 *>(&addr);
		memset(&addr, 0, sizeof(struct sockaddr_storage));
		addr6->sin6_family = AF_INET6;
		addr6->sin6_port = htons(adr_.port);
		addr6->sin6_scope_id = adr_.scopeId;
		memcpy(&addr6->sin6_addr, &adr_.ip.ip[0], 16);
		return sizeof(struct sockaddr_in6);
	};

	Socket::Socket() {
		this_ = new Socket_();
		this_->value = INVALID_SOCKET;
		ipAddress = nullptr;
		ipAddressIs6 = false;
	};

	Socket::~Socket() {
		close();
		delete this_;
	};

	Socket::operator bool() const {
		return (this_->value != INVALID_SOCKET);
	};

	bool Socket::openClient(IPAddress4 &adr_) {
		struct sockaddr_storage addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		int addrLn = socketAddress_(adr_, addr);
		return socketOpen_(this_, ipAddress, ipAddressIs6, (struct sockaddr *)&addr, addrLn, false);
	};

	bool Socket::openClient(IPAddress6 &adr_) {
		struct sockaddr_storage addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		int addrLn = socketAddress_(adr_, addr);
		return socketOpen_(this_, ipAddress, ipAddressIs6, (struct sockaddr *)&addr, addrLn, false);
	};

	bool Socket::openServer(IPAddress4 &adr_) {
		struct sockaddr_storage addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		int addrLn = socketAddress_(adr_, addr);
		return socketOpen_(this_, ipAddress, ipAddressIs6, (struct sockaddr *)&addr, addrLn, true);
	};

	bool Socket::openServer(IPAddress6 &adr_) {
		struct sockaddr_storage addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		int addrLn = socketAddress_(adr_, addr);
		return socketOpen_(this_, ipAddress, ipAddressIs6, (struct sockaddr *)&addr, addrLn, true);
	};

	bool Socket::listen(uint16_t queue_) {
		if (this_->value == INVALID_SOCKET) {
			return false;
		};
		return (::listen(this_->value, queue_) == 0);
	};

	bool Socket::accept(Socket &socket_) {
		struct sockaddr_storage addr;
		socklen_t addrLn;
		SOCKET value;

		socket_.close();

		if (this_->value == INVALID_SOCKET) {
			return false;
		};

		if (!ipAddress) {
			return false;
		};

		addrLn = sizeof(struct sockaddr_storage);
		value = ::accept(this_->value, (struct sockaddr *)&addr, &addrLn);
		if (value == INVALID_SOCKET) {
			return false;
		};

		SetHandleInformation(reinterpret_cast<HANDLE>(value), HANDLE_FLAG_INHERIT, 0);

		socket_.this_->value = value;
		socket_.ipAddress = socketNewIPAddress_((struct sockaddr *)&addr, socket_.ipAddressIs6);
		return true;
	};

	void Socket::shutdown() {
		if (this_->value == INVALID_SOCKET) {
			return;
		};

		::shutdown(this_->value, SD_BOTH);
	};

	void Socket::close() {
		if (this_->value != INVALID_SOCKET) {
			Socket::shutdown();
			closesocket(this_->value);
			this_->value = INVALID_SOCKET;
		};

		if (ipAddress) {
			if (ipAddressIs6) {
				delete reinterpret_cast<IPAddress6 *>(ipAddress);
			} else {
				delete reinterpret_cast<IPAddress4 *>(ipAddress);
			};
		};

		ipAddress = nullptr;
		ipAddressIs6 = false;
	};

	size_t Socket::read(void *output, size_t ln) {
		int recvLn;
		if (this_->value == INVALID_SOCKET) {
			return 0;
		};

		if (ln > INT_MAX) {
			ln = INT_MAX;
		};

		recvLn = recv(this_->value, (char *)output, static_cast<int>(ln), 0);
		if (recvLn == SOCKET_ERROR) {
			return 0;
		};
		return recvLn;
	};

	size_t Socket::write(const void *input, size_t ln) {
		const char *buffer = static_cast<const char *>(input);
		size_t idx = 0;
		size_t toSend;
		int sendLn;

		if (this_->value == INVALID_SOCKET) {
			return 0;
		};

		if (ln == 0) { // keep alive
			send(this_->value, buffer, 0, 0);
			return 0;
		};

		while (idx < ln) {
			toSend = ln - idx;
			if (toSend > INT_MAX) {
				toSend = INT_MAX;
			};
			sendLn = send(this_->value, &buffer[idx], static_cast<int>(toSend), 0);
			if (sendLn == SOCKET_ERROR) {
				break;
			};
			idx += sendLn;
		};
		return idx;
	};

	int Socket::waitToWrite(uint32_t microSeconds) {
		if (this_->value == INVALID_SOCKET) {
			return -1;
		};

		return socketWait_(this_->value, true, microSeconds);
	};

	int Socket::waitToRead(uint32_t microSeconds) {
		if (this_->value == INVALID_SOCKET) {
			return -1;
		};

		return socketWait_(this_->value, false, microSeconds);
	};

	bool Socket::openClientX(const String &adr_) {
		close();

		if (!Network::isValid()) {
			return false;
		};

		return socketOpenX_(this_, ipAddress, ipAddressIs6, adr_, false);
	};

	bool Socket::openServerX(const String &adr_) {
		close();

		if (!Network::isValid()) {
			return false;
		};

		return socketOpenX_(this_, ipAddress, ipAddressIs6, adr_, true);
	};

};

#endif
