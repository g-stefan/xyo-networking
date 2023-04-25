// Networking
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

#ifdef XYO_OS_WINDOWS

#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#	include <winsock2.h>
#	include <ws2tcpip.h>

#	include <XYO/Networking/Network.hpp>
#	include <XYO/Networking/Socket.hpp>
#	include <XYO/Networking/URL.hpp>

namespace XYO::Networking {

	class Socket_ {
		public:
			SOCKET value;
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
		struct sockaddr_in addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		ipAddress = reinterpret_cast<IPAddress_ *>(new IPAddress4());
		ipAddressIs6 = false;

		memset(&addr, 0, sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(adr_.port);
		addr.sin_addr.s_addr = adr_.ip.toU32();

		this_->value = socket(AF_INET, SOCK_STREAM, 0);
		if (this_->value != INVALID_SOCKET) {
			if (connect(this_->value, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == 0) {
				(reinterpret_cast<IPAddress4 *>(ipAddress))->copy(adr_);
				return true;
			};
			closesocket(this_->value);
			this_->value = INVALID_SOCKET;
		};

		delete reinterpret_cast<IPAddress4 *>(ipAddress);
		ipAddress = nullptr;
		ipAddressIs6 = false;

		return false;
	};

	bool Socket::openClient(IPAddress6 &adr_) {
		struct sockaddr_in6 addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		ipAddress = reinterpret_cast<IPAddress_ *>(new IPAddress6());
		ipAddressIs6 = true;

		memset(&addr, 0, sizeof(struct sockaddr_in6));
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(adr_.port);

		int k;
		for (k = 0; k < 16; ++k) {
			addr.sin6_addr.s6_addr[k] = (reinterpret_cast<IPAddress6 *>(ipAddress))->ip.ip[k];
		};

		this_->value = socket(AF_INET6, SOCK_STREAM, 0);
		if (this_->value != INVALID_SOCKET) {
			if (connect(this_->value, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6)) == 0) {
				(reinterpret_cast<IPAddress6 *>(ipAddress))->copy(adr_);
				return true;
			};
			closesocket(this_->value);
			this_->value = INVALID_SOCKET;
		};

		delete reinterpret_cast<IPAddress6 *>(ipAddress);
		ipAddress = nullptr;
		ipAddressIs6 = false;

		return false;
	};

	bool Socket::openServer(IPAddress4 &adr_) {
		struct sockaddr_in addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		ipAddress = reinterpret_cast<IPAddress_ *>(new IPAddress4());
		ipAddressIs6 = false;

		memset(&addr, 0, sizeof(struct sockaddr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(adr_.port);
		addr.sin_addr.s_addr = adr_.ip.toU32();

		this_->value = socket(AF_INET, SOCK_STREAM, 0);
		if (this_->value != INVALID_SOCKET) {
			if (bind(this_->value, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == 0) {
				(reinterpret_cast<IPAddress4 *>(ipAddress))->copy(adr_);
				return true;
			};
			closesocket(this_->value);
			this_->value = INVALID_SOCKET;
		};

		delete reinterpret_cast<IPAddress4 *>(ipAddress);
		ipAddress = nullptr;
		ipAddressIs6 = false;

		return false;
	};

	bool Socket::openServer(IPAddress6 &adr_) {
		struct sockaddr_in6 addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		ipAddress = reinterpret_cast<IPAddress_ *>(new IPAddress6());
		ipAddressIs6 = true;

		memset(&addr, 0, sizeof(struct sockaddr_in6));
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(adr_.port);

		int k;
		for (k = 0; k < 16; ++k) {
			addr.sin6_addr.s6_addr[k] = (reinterpret_cast<IPAddress6 *>(ipAddress))->ip.ip[k];
		};

		this_->value = socket(AF_INET6, SOCK_STREAM, 0);
		if (this_->value != INVALID_SOCKET) {
			if (bind(this_->value, (struct sockaddr *)&addr, sizeof(struct sockaddr_in6)) == 0) {
				(reinterpret_cast<IPAddress6 *>(ipAddress))->copy(adr_);
				return true;
			};
			closesocket(this_->value);
			this_->value = INVALID_SOCKET;
		};

		delete reinterpret_cast<IPAddress6 *>(ipAddress);
		ipAddress = nullptr;
		ipAddressIs6 = false;

		return false;
	};

	bool Socket::listen(uint16_t queue_) {
		if (this_->value == INVALID_SOCKET) {
			return false;
		};
		return ((::listen(this_->value, queue_)) != INVALID_SOCKET);
	};

	bool Socket::accept(Socket &socket_) {
		socket_.close();

		socklen_t _addrlen;

		if (this_->value == INVALID_SOCKET) {
			return false;
		};

		if (!ipAddress) {
			return false;
		};

		if (ipAddressIs6) {
			struct sockaddr_in6 addr;
			memset(&addr, 0, sizeof(struct sockaddr_in6));
			_addrlen = sizeof(struct sockaddr_in6);
			addr.sin6_family = AF_INET6;
			socket_.this_->value = ::accept(this_->value, (struct sockaddr *)&addr, &_addrlen);
			if (socket_.this_->value != INVALID_SOCKET) {
				socket_.ipAddress = reinterpret_cast<IPAddress_ *>(new IPAddress6());
				socket_.ipAddressIs6 = true;
				(reinterpret_cast<IPAddress6 *>(socket_.ipAddress))->port = addr.sin6_port;
				int k;
				for (k = 0; k < 16; ++k) {
					(reinterpret_cast<IPAddress6 *>(socket_.ipAddress))->ip.ip[k] = addr.sin6_addr.s6_addr[k];
				};
				return true;
			};
		} else {
			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(struct sockaddr_in));
			_addrlen = sizeof(struct sockaddr_in);
			addr.sin_family = AF_INET;
			socket_.this_->value = ::accept(this_->value, (struct sockaddr *)&addr, &_addrlen);
			if (socket_.this_->value != INVALID_SOCKET) {
				socket_.ipAddress = reinterpret_cast<IPAddress_ *>(new IPAddress4());
				socket_.ipAddressIs6 = false;
				(reinterpret_cast<IPAddress4 *>(socket_.ipAddress))->port = addr.sin_port;
				(reinterpret_cast<IPAddress4 *>(socket_.ipAddress))->ip.fromU32(addr.sin_addr.s_addr);
				return true;
			};
		};

		return false;
	};

	void Socket::shutdown() {
		if (this_->value = INVALID_SOCKET) {
			return;
		};

		::shutdown(this_->value, 2);
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
		long int recvLn;
		if (this_->value == INVALID_SOCKET) {
			return 0;
		};

		recvLn = recv(this_->value, (char *)output, ln, 0);
		if (recvLn == SOCKET_ERROR) {
			return 0;
		};
		return (recvLn);
	};

	size_t Socket::write(const void *input, size_t ln_) {
		size_t idx = 0;
		long int ln = ln_;
		long int sndln;

		if (this_->value == INVALID_SOCKET) {
			return 0;
		};
		if (ln_ == 0) { // keep alive
			waitToWrite(0);
			sndln = send(this_->value, (const char *)input, ln_, 0);
			if (sndln == SOCKET_ERROR) {
				return 0;
			};
			return sndln;
		};
		do {
			waitToWrite(0);
			sndln = send(this_->value, &(((const char *)input)[idx]), ln, 0);
			if (sndln == SOCKET_ERROR) {
				return idx;
			};
			ln -= sndln;
			idx += sndln;
		} while (ln > 0);
		return idx;
	};

	int Socket::waitToWrite(uint32_t microSeconds) {
		int ret;
		fd_set sock_set;
		struct timeval timev;

		if (this_->value == INVALID_SOCKET) {
			return -1;
		}

		timev.tv_sec = (microSeconds / 3600);
		timev.tv_usec = (microSeconds % 3600);
		FD_ZERO(&sock_set);
		FD_SET(this_->value, &sock_set);
		ret = select(1, nullptr, &sock_set, nullptr, &timev);
		if (ret == SOCKET_ERROR) {
			return -1;
		}
		if (ret == 0) {
			return 0;
		}
		return 1;
	};

	int Socket::waitToRead(uint32_t microSeconds) {
		int ret;
		fd_set sock_set;
		struct timeval timev;

		if (this_->value == INVALID_SOCKET) {
			return -1;
		}

		timev.tv_sec = (microSeconds / 3600);
		timev.tv_usec = (microSeconds % 3600);
		FD_ZERO(&sock_set);
		FD_SET(this_->value, &sock_set);
		ret = select(1, &sock_set, nullptr, nullptr, &timev);
		if (ret == SOCKET_ERROR) {
			return -1;
		}
		if (ret == 0) {
			return 0;
		}
		return 1;
	};

	bool Socket::openClientX(const String &adr_) {
		if (!Network::isValid()) {
			return false;
		};

		int ip0, ip1, ip2, ip3, port;
		if (sscanf(adr_, "%d.%d.%d.%d:%d", &ip0, &ip1, &ip2, &ip3, &port) == 5) {
			IPAddress4 adr;
			adr.ip.ip[0] = ip0;
			adr.ip.ip[1] = ip1;
			adr.ip.ip[2] = ip2;
			adr.ip.ip[3] = ip3;
			adr.port = port;
			return openClient(adr);
		};

		String urlSchemeName;
		String urlHostAndPort;
		String urlAddress;
		String urlPort;

		if (!URL::getSchemeName(adr_, urlSchemeName)) {
			urlSchemeName = "none";
		};
		if (!URL::getHostNameAndPort(adr_, urlHostAndPort)) {
			urlHostAndPort = adr_;
		};
		urlAddress = urlHostAndPort;
		if (!urlHostAndPort.split2FromEnd(":", urlAddress, urlPort)) {
			if (urlSchemeName.matchAscii("http")) {
				urlPort = "80";
			} else if (urlSchemeName.matchAscii("https")) {
				urlPort = "443";
			} else if (urlSchemeName.matchAscii("ftp")) {
				urlPort = "20";
			} else if (urlSchemeName.matchAscii("ssh")) {
				urlPort = "22";
			} else if (urlSchemeName.matchAscii("telnet")) {
				urlPort = "23";
			} else {
				return false;
			};
		};
		if (urlAddress[0] == '[') {
			urlAddress = urlAddress.substring(1, urlAddress.length() - 2);
		};
		if (sscanf(urlPort, "%d", &port) != 1) {
			return false;
		};

		struct addrinfo hints;
		struct addrinfo *result;
		struct addrinfo *scan;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = 0;
		hints.ai_protocol = 0;
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;

		if (getaddrinfo(urlAddress, nullptr, &hints, &result) != 0) {
			return false;
		};
		for (scan = result; scan != NULL; scan = scan->ai_next) {
			if (scan->ai_family == AF_INET) {
				IPAddress4 adr;
				adr.port = port;
				adr.ip.fromU32(((struct sockaddr_in *)(scan->ai_addr))->sin_addr.s_addr);
				if (openClient(adr)) {
					freeaddrinfo(result);
					return true;
				};
			};
			if (scan->ai_family == AF_INET6) {
				IPAddress6 adr;
				adr.port = port;

				int k;
				for (k = 0; k < 16; ++k) {
					adr.ip.ip[k] = ((struct sockaddr_in6 *)(scan->ai_addr))->sin6_addr.s6_addr[k];
				};

				if (openClient(adr)) {
					freeaddrinfo(result);
					return true;
				};
			};
		};

		freeaddrinfo(result);

		return false;
	};

	bool Socket::openServerX(const String &adr_) {

		if (!Network::isValid()) {
			return false;
		};

		int ip0, ip1, ip2, ip3, port;
		if (sscanf(adr_, "%d.%d.%d.%d:%d", &ip0, &ip1, &ip2, &ip3, &port) == 5) {
			IPAddress4 adr;
			adr.ip.ip[0] = ip0;
			adr.ip.ip[1] = ip1;
			adr.ip.ip[2] = ip2;
			adr.ip.ip[3] = ip3;
			adr.port = port;
			return openServer(adr);
		};

		String urlSchemeName;
		String urlHostAndPort;
		String urlAddress;
		String urlPort;

		if (!URL::getSchemeName(adr_, urlSchemeName)) {
			urlSchemeName = "none";
		};
		if (!URL::getHostNameAndPort(adr_, urlHostAndPort)) {
			urlHostAndPort = adr_;
		};
		urlAddress = urlHostAndPort;
		if (!urlHostAndPort.split2FromEnd(":", urlAddress, urlPort)) {
			if (urlSchemeName.matchAscii("http")) {
				urlPort = "80";
			} else if (urlSchemeName.matchAscii("https")) {
				urlPort = "443";
			} else if (urlSchemeName.matchAscii("ftp")) {
				urlPort = "20";
			} else if (urlSchemeName.matchAscii("ssh")) {
				urlPort = "22";
			} else if (urlSchemeName.matchAscii("telnet")) {
				urlPort = "23";
			} else {
				return false;
			};
		};
		if (urlAddress[0] == '[') {
			urlAddress = urlAddress.substring(1, urlAddress.length() - 2);
		};
		if (sscanf(urlPort, "%d", &port) != 1) {
			return false;
		};

		struct addrinfo hints;
		struct addrinfo *result;
		struct addrinfo *scan;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = 0;
		hints.ai_canonname = NULL;
		hints.ai_addr = NULL;
		hints.ai_next = NULL;

		if (getaddrinfo(urlAddress, nullptr, &hints, &result) != 0) {
			return false;
		};
		for (scan = result; scan != NULL; scan = scan->ai_next) {
			if (scan->ai_family == AF_INET) {
				IPAddress4 adr;
				adr.port = port;
				adr.ip.fromU32(((struct sockaddr_in *)(scan->ai_addr))->sin_addr.s_addr);
				if (openServer(adr)) {
					freeaddrinfo(result);
					return true;
				};
			};
			if (scan->ai_family == AF_INET6) {
				IPAddress6 adr;
				adr.port = port;

				int k;
				for (k = 0; k < 16; ++k) {
					adr.ip.ip[k] = ((struct sockaddr_in6 *)(scan->ai_addr))->sin6_addr.s6_addr[k];
				};

				if (openServer(adr)) {
					freeaddrinfo(result);
					return true;
				};
			};
		};

		freeaddrinfo(result);

		return false;
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

#endif
