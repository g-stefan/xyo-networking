// Networking
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_NETWORKING_DEPENDENCY_HPP
#	include <XYO/Networking/Dependency.hpp>
#endif

#ifdef XYO_PLATFORM_OS_LINUX

#	include <unistd.h>
#	include <errno.h>
#	include <poll.h>
#	include <time.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>

#	include <XYO/Networking/Network.hpp>
#	include <XYO/Networking/Socket.hpp>
#	include <XYO/Networking/URL.hpp>

namespace XYO::Networking {

#	define INVALID_SOCKET -1
#	define SOCKET_ERROR -1
#	define closesocket ::close

	class Socket_ {
		public:
			int value;
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

	// poll is not limited by FD_SETSIZE (1024) like select
	static int socketWait_(int value, short events, uint32_t microSeconds) {
		struct pollfd pollFd;
		struct timespec timeout;
		struct timespec end;
		struct timespec now;
		int ret;

		pollFd.fd = value;
		pollFd.events = events;
		pollFd.revents = 0;

		timeout.tv_sec = (microSeconds / (1000 * 1000));
		timeout.tv_nsec = (microSeconds % (1000 * 1000)) * 1000;

		clock_gettime(CLOCK_MONOTONIC, &end);
		end.tv_sec += timeout.tv_sec;
		end.tv_nsec += timeout.tv_nsec;
		if (end.tv_nsec >= (1000 * 1000 * 1000)) {
			end.tv_sec++;
			end.tv_nsec -= (1000 * 1000 * 1000);
		};

		for (;;) {
			ret = ppoll(&pollFd, 1, &timeout, nullptr);
			if (ret > 0) {
				if (pollFd.revents & POLLNVAL) {
					return -1;
				};
				return 1;
			};
			if (ret == 0) {
				return 0;
			};
			if (errno != EINTR) {
				return -1;
			};

			clock_gettime(CLOCK_MONOTONIC, &now);
			timeout.tv_sec = end.tv_sec - now.tv_sec;
			timeout.tv_nsec = end.tv_nsec - now.tv_nsec;
			if (timeout.tv_nsec < 0) {
				timeout.tv_sec--;
				timeout.tv_nsec += (1000 * 1000 * 1000);
			};
			if (timeout.tv_sec < 0) {
				return 0;
			};
		};
	};

	static bool socketOpen_(Socket_ *this_, IPAddress_ *&ipAddress, bool &ipAddressIs6, const struct sockaddr *addr, socklen_t addrLn, bool isServer) {
		int option;

		this_->value = socket(addr->sa_family, SOCK_STREAM | SOCK_CLOEXEC, 0);
		if (this_->value == INVALID_SOCKET) {
			return false;
		};

		if (isServer) {
			option = 1;
			setsockopt(this_->value, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
			if (addr->sa_family == AF_INET6) {
				// Accept IPv4 connections too, same as on Windows
				option = 0;
				setsockopt(this_->value, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option));
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
			if (errno == EINTR) {
				// Connection continues in background, wait for it
				struct pollfd pollFd;
				int ret;
				pollFd.fd = this_->value;
				pollFd.events = POLLOUT;
				pollFd.revents = 0;
				do {
					ret = poll(&pollFd, 1, -1);
				} while ((ret < 0) && (errno == EINTR));
				if (ret > 0) {
					socklen_t optionLn = sizeof(option);
					option = 0;
					if (getsockopt(this_->value, SOL_SOCKET, SO_ERROR, &option, &optionLn) == 0) {
						if (option == 0) {
							ipAddress = socketNewIPAddress_(addr, ipAddressIs6);
							return true;
						};
					};
				};
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
			if (socketOpen_(this_, ipAddress, ipAddressIs6, scan->ai_addr, scan->ai_addrlen, isServer)) {
				freeaddrinfo(result);
				return true;
			};
		};

		freeaddrinfo(result);
		return false;
	};

	static socklen_t socketAddress_(IPAddress4 &adr_, struct sockaddr_storage &addr) {
		struct sockaddr_in *addr4 = reinterpret_cast<struct sockaddr_in *>(&addr);
		memset(&addr, 0, sizeof(struct sockaddr_storage));
		addr4->sin_family = AF_INET;
		addr4->sin_port = htons(adr_.port);
		memcpy(&addr4->sin_addr, &adr_.ip.ip[0], 4);
		return sizeof(struct sockaddr_in);
	};

	static socklen_t socketAddress_(IPAddress6 &adr_, struct sockaddr_storage &addr) {
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

		socklen_t addrLn = socketAddress_(adr_, addr);
		return socketOpen_(this_, ipAddress, ipAddressIs6, (struct sockaddr *)&addr, addrLn, false);
	};

	bool Socket::openClient(IPAddress6 &adr_) {
		struct sockaddr_storage addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		socklen_t addrLn = socketAddress_(adr_, addr);
		return socketOpen_(this_, ipAddress, ipAddressIs6, (struct sockaddr *)&addr, addrLn, false);
	};

	bool Socket::openServer(IPAddress4 &adr_) {
		struct sockaddr_storage addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		socklen_t addrLn = socketAddress_(adr_, addr);
		return socketOpen_(this_, ipAddress, ipAddressIs6, (struct sockaddr *)&addr, addrLn, true);
	};

	bool Socket::openServer(IPAddress6 &adr_) {
		struct sockaddr_storage addr;
		close();

		if (!Network::isValid()) {
			return false;
		};

		socklen_t addrLn = socketAddress_(adr_, addr);
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
		int value;

		socket_.close();

		if (this_->value == INVALID_SOCKET) {
			return false;
		};

		if (!ipAddress) {
			return false;
		};

		do {
			addrLn = sizeof(struct sockaddr_storage);
			value = accept4(this_->value, (struct sockaddr *)&addr, &addrLn, SOCK_CLOEXEC);
		} while ((value == INVALID_SOCKET) && (errno == EINTR));

		if (value == INVALID_SOCKET) {
			return false;
		};

		socket_.this_->value = value;
		socket_.ipAddress = socketNewIPAddress_((struct sockaddr *)&addr, socket_.ipAddressIs6);
		return true;
	};

	void Socket::shutdown() {
		if (this_->value == INVALID_SOCKET) {
			return;
		};

		::shutdown(this_->value, SHUT_RDWR);
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
		ssize_t recvLn;
		if (this_->value == INVALID_SOCKET) {
			return 0;
		};

		do {
			recvLn = recv(this_->value, output, ln, 0);
		} while ((recvLn == SOCKET_ERROR) && (errno == EINTR));

		if (recvLn == SOCKET_ERROR) {
			return 0;
		};
		return recvLn;
	};

	size_t Socket::write(const void *input, size_t ln) {
		const char *buffer = static_cast<const char *>(input);
		size_t idx = 0;
		ssize_t sendLn;

		if (this_->value == INVALID_SOCKET) {
			return 0;
		};

		// MSG_NOSIGNAL - peer closed connection must not raise SIGPIPE

		if (ln == 0) { // keep alive
			do {
				sendLn = send(this_->value, buffer, 0, MSG_NOSIGNAL);
			} while ((sendLn == SOCKET_ERROR) && (errno == EINTR));
			return 0;
		};

		while (idx < ln) {
			sendLn = send(this_->value, &buffer[idx], ln - idx, MSG_NOSIGNAL);
			if (sendLn == SOCKET_ERROR) {
				if (errno == EINTR) {
					continue;
				};
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

		return socketWait_(this_->value, POLLOUT, microSeconds);
	};

	int Socket::waitToRead(uint32_t microSeconds) {
		if (this_->value == INVALID_SOCKET) {
			return -1;
		};

		return socketWait_(this_->value, POLLIN, microSeconds);
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
