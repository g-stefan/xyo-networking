// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

#include <XYO/Networking.hpp>

// Only to find a link-local address for testing
#ifdef XYO_PLATFORM_OS_WINDOWS
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#	include <winsock2.h>
#	include <ws2tcpip.h>
#	include <iphlpapi.h>
#	pragma comment(lib, "iphlpapi.lib")
#endif
#ifdef XYO_PLATFORM_OS_LINUX
#	include <sys/types.h>
#	include <ifaddrs.h>
#	include <netinet/in.h>
#endif

using namespace XYO::Networking;

static void check(bool value, const char *message) {
	if (!value) {
		throw std::runtime_error(message);
	};
};

static bool isEqual(const String &value, const char *expected) {
	return (strcmp(value.value(), expected) == 0);
};

void testURL() {
	String out;

	check(URL::getQuery("http://h/p?a=1", out) && isEqual(out, "a=1"), "URL::getQuery without fragment");
	check(URL::getQuery("http://h/p?a=1#f", out) && isEqual(out, "a=1"), "URL::getQuery with fragment");
	check(URL::getQuery("http://h?a=1", out) && isEqual(out, "a=1"), "URL::getQuery without path");
	check(!URL::getQuery("http://h/p#f?a=1", out), "URL::getQuery inside fragment");
	check(!URL::getQuery("http://h/p", out), "URL::getQuery missing");

	check(URL::getHostNameAndPort("http://user:pw@host", out) && isEqual(out, "host"), "URL::getHostNameAndPort user without path");
	check(URL::getHostNameAndPort("http://user:pw@host:8080/p?x", out) && isEqual(out, "host:8080"), "URL::getHostNameAndPort user with path");
	check(URL::getHostNameAndPort("http://host?x=@", out) && isEqual(out, "host"), "URL::getHostNameAndPort with query");
	check(URL::getHostNameAndPort("http://[::1]:80/", out) && isEqual(out, "[::1]:80"), "URL::getHostNameAndPort IPv6");

	check(URL::getUsernameAndPassword("http://user:pw@host", out) && isEqual(out, "user:pw"), "URL::getUsernameAndPassword without path");
	check(!URL::getUsernameAndPassword("http://host/a@b", out), "URL::getUsernameAndPassword @ in path");

	check(URL::getPathAndFileName("http://h/p/f.txt?x#y", out) && isEqual(out, "/p/f.txt"), "URL::getPathAndFileName with query");
	check(URL::getPathAndFileName("http://h/p#f", out) && isEqual(out, "/p"), "URL::getPathAndFileName with fragment");
	check(URL::getPathAndFileName("http://h", out) && isEqual(out, "/"), "URL::getPathAndFileName empty");

	check(URL::getSchemeName("http://x", out) && isEqual(out, "http"), "URL::getSchemeName");
	check(URL::getSchemeName("svn+ssh://x", out) && isEqual(out, "svn+ssh"), "URL::getSchemeName with +");
	check(!URL::getSchemeName("host/p?r=http://x", out), "URL::getSchemeName :// inside query");
	check(!URL::getSchemeName("1http://x", out), "URL::getSchemeName starts with digit");
	check(!URL::getSchemeName("http:/x", out), "URL::getSchemeName missing /");
	check(!URL::getSchemeName("http:/", out), "URL::getSchemeName short");
	check(!URL::getHostNameAndPort("host/p?r=http://x", out), "URL::getHostNameAndPort :// inside query");

	check(isEqual(URL::encodeComponent("a-b_c d"), "a-b_c%20d"), "URL::encodeComponent");
	check(isEqual(URL::decodeComponent("a-b_c%20d"), "a-b_c d"), "URL::decodeComponent");
	check(isEqual(URL::decodeComponent("%41%42"), "AB"), "URL::decodeComponent sequence");
	check(isEqual(URL::decodeComponent("100%"), "100%"), "URL::decodeComponent incomplete at end");
	check(isEqual(URL::decodeComponent("%4"), "%4"), "URL::decodeComponent incomplete");
	check(isEqual(URL::decodeComponent("%zz%4g"), "%zz%4g"), "URL::decodeComponent invalid hex");

	printf("URL: ok\n");
};

void testIP4() {
	IP4 ip;
	IP4 ipCopy;
	uint32_t value;

	ip.ip[0] = 192;
	ip.ip[1] = 168;
	ip.ip[2] = 1;
	ip.ip[3] = 2;
	value = ip.toU32();
	check(memcmp(&value, &ip.ip[0], 4) == 0, "IP4::toU32 network byte order");
	ipCopy.fromU32(value);
	check(memcmp(&ipCopy.ip[0], &ip.ip[0], 4) == 0, "IP4::fromU32");

	printf("IP4: ok\n");
};

static bool isLinkLocal(const uint8_t *ip) {
	return (ip[0] == 0xFE) && ((ip[1] & 0xC0) == 0x80);
};

static bool findLinkLocal(IPAddress6 &address) {
#ifdef XYO_PLATFORM_OS_LINUX
	struct ifaddrs *list;
	struct ifaddrs *scan;
	bool found = false;
	if (getifaddrs(&list) != 0) {
		return false;
	};
	for (scan = list; scan != nullptr; scan = scan->ifa_next) {
		if (scan->ifa_addr == nullptr) {
			continue;
		};
		if (scan->ifa_addr->sa_family != AF_INET6) {
			continue;
		};
		const struct sockaddr_in6 *addr6 = reinterpret_cast<const struct sockaddr_in6 *>(scan->ifa_addr);
		if (isLinkLocal(addr6->sin6_addr.s6_addr) && (addr6->sin6_scope_id != 0)) {
			memcpy(&address.ip.ip[0], &addr6->sin6_addr, 16);
			address.scopeId = addr6->sin6_scope_id;
			found = true;
			break;
		};
	};
	freeifaddrs(list);
	return found;
#endif
#ifdef XYO_PLATFORM_OS_WINDOWS
	ULONG size = 16 * 1024;
	IP_ADAPTER_ADDRESSES *list;
	IP_ADAPTER_ADDRESSES *scan;
	IP_ADAPTER_UNICAST_ADDRESS *unicast;
	bool found = false;
	ULONG ret;
	for (;;) {
		list = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(malloc(size));
		ret = GetAdaptersAddresses(AF_INET6, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, nullptr, list, &size);
		if (ret != ERROR_BUFFER_OVERFLOW) {
			break;
		};
		free(list);
	};
	if (ret == NO_ERROR) {
		for (scan = list; (scan != nullptr) && (!found); scan = scan->Next) {
			if (scan->OperStatus != IfOperStatusUp) {
				continue;
			};
			for (unicast = scan->FirstUnicastAddress; unicast != nullptr; unicast = unicast->Next) {
				const struct sockaddr_in6 *addr6 = reinterpret_cast<const struct sockaddr_in6 *>(unicast->Address.lpSockaddr);
				if (isLinkLocal(addr6->sin6_addr.s6_addr) && (addr6->sin6_scope_id != 0) && (unicast->DadState == IpDadStatePreferred)) {
					memcpy(&address.ip.ip[0], &addr6->sin6_addr, 16);
					address.scopeId = addr6->sin6_scope_id;
					found = true;
					break;
				};
			};
		};
	};
	free(list);
	return found;
#endif
};

void testLinkLocal() {
	IPAddress6 address;
	IPAddress6 peerAddress;
	Socket server;
	Socket client;
	Socket peer;
	char text[128];
	uint16_t port;

	if (!findLinkLocal(address)) {
		printf("Link-local: skipped (no link-local address)\n");
		return;
	};

	for (port = 27100; port < 27300; ++port) {
		address.port = port;
		if (server.openServer(address)) {
			if (server.listen(16)) {
				break;
			};
		};
	};
	check(server, "link-local server");

#ifdef XYO_PLATFORM_OS_LINUX
	// Linux rejects link-local addresses without interface
	uint32_t scopeId = address.scopeId;
	address.scopeId = 0;
	check(!client.openClient(address), "link-local connect without scopeId");
	address.scopeId = scopeId;
#endif

	check(client.openClient(address), "link-local connect");
	check(server.accept(peer), "link-local accept");
	check(peer.getIPAddress6(peerAddress), "link-local peer address");
	check(peerAddress.scopeId == address.scopeId, "link-local peer scopeId");
	check(client.getIPAddress6(peerAddress) && (peerAddress.scopeId == address.scopeId), "link-local client scopeId");
	client.close();
	peer.close();

	const uint8_t *ip = &address.ip.ip[0];
	snprintf(text, sizeof(text), "[%x:%x:%x:%x:%x:%x:%x:%x%%%u]:%u",
	         (ip[0] << 8) | ip[1], (ip[2] << 8) | ip[3], (ip[4] << 8) | ip[5], (ip[6] << 8) | ip[7],
	         (ip[8] << 8) | ip[9], (ip[10] << 8) | ip[11], (ip[12] << 8) | ip[13], (ip[14] << 8) | ip[15],
	         (unsigned)address.scopeId, (unsigned)address.port);
	check(client.openClientX(text), "link-local connect by name");
	check(server.accept(peer), "link-local accept by name");
	check(client.getIPAddress6(peerAddress) && (peerAddress.scopeId == address.scopeId), "link-local client scopeId by name");

	printf("Link-local: ok (%s)\n", text);
};

static bool openServer(Socket &server, char *address, size_t addressSize, const char *format) {
	uint16_t port;
	for (port = 27100; port < 27300; ++port) {
		snprintf(address, addressSize, format, port);
		if (server.openServerX(address)) {
			if (server.listen(128)) {
				return true;
			};
		};
	};
	return false;
};

void testAddress() {
	Socket server;
	Socket client;
	Socket peer;
	char address[128];
	char clientAddress[128];
	uint16_t port;

	check(!client.openClientX("127.0.0.1:99999"), "port out of range");
	check(!client.openClientX("127.0.0.1:80x"), "port not numeric");
	check(!client.openClientX("[::1"), "IPv6 missing bracket");
	check(!client.openClientX("127.0.0.1"), "missing port");

	check(openServer(server, address, sizeof(address), "127.0.0.1:%u"), "IPv4 server");
	sscanf(address, "127.0.0.1:%hu", &port);

	snprintf(clientAddress, sizeof(clientAddress), "http://user:pw@127.0.0.1:%u", port);
	check(client.openClientX(clientAddress), "connect with user info");
	check(server.accept(peer), "accept");
	check(client.isIPAddress4(), "client IPv4");

	IPAddress4 ipAddress4;
	check(client.getIPAddress4(ipAddress4), "client get IPv4");
	check((ipAddress4.ip.ip[0] == 127) && (ipAddress4.ip.ip[3] == 1) && (ipAddress4.port == port), "client IPv4 address");
	check(peer.getIPAddress4(ipAddress4), "peer get IPv4");
	check((ipAddress4.ip.ip[0] == 127) && (ipAddress4.ip.ip[3] == 1), "peer IPv4 address");

	client.close();
	peer.close();
	server.close();

	if (openServer(server, address, sizeof(address), "[::1]:%u")) {
		sscanf(address, "[::1]:%hu", &port);
		snprintf(clientAddress, sizeof(clientAddress), "http://[::1]:%u/path?q=1", port);
		check(client.openClientX(clientAddress), "connect IPv6");
		check(server.accept(peer), "accept IPv6");
		check(client.isIPAddress6() && peer.isIPAddress6(), "IPv6 address");
		printf("Address: ok\n");
		return;
	};

	printf("Address: ok (IPv6 loopback not available)\n");
};

void testManySockets() {
	const size_t count = 1100;
	Socket server;
	Socket *clients;
	Socket *peers;
	char address[128];
	char buffer[1024];
	size_t k;

	check(openServer(server, address, sizeof(address), "127.0.0.1:%u"), "server");

	clients = new Socket[count];
	peers = new Socket[count];

	try {
		for (k = 0; k < count; ++k) {
			if (!clients[k].openClientX(address)) {
				snprintf(buffer, sizeof(buffer), "connect failed at socket %u", (unsigned)(k * 2 + 1));
				check(false, buffer);
			};
			check(server.accept(peers[k]), "accept");
		};

		// Last sockets have the highest values, above 1024 on Linux
		for (k = count - 8; k < count; ++k) {
			check(clients[k].waitToWrite(0) == 1, "waitToWrite");
			check(clients[k].write("ping", 4) == 4, "write");
			check(peers[k].waitToRead(1000 * 1000) == 1, "waitToRead");
			check(peers[k].read(buffer, 4) == 4, "read");
			check(memcmp(buffer, "ping", 4) == 0, "read data");
		};
		check(peers[0].waitToRead(1000) == 0, "waitToRead timeout");

		// Write to a connection closed by peer must fail, not kill the process
		peers[count - 1].close();
		memset(buffer, 0, sizeof(buffer));
		for (k = 0; k < 100; ++k) {
			if (clients[count - 1].write(buffer, sizeof(buffer)) != sizeof(buffer)) {
				break;
			};
			clients[count - 1].waitToRead(10 * 1000);
		};
		check(k < 100, "write to closed peer");
	} catch (...) {
		delete[] clients;
		delete[] peers;
		throw;
	};

	delete[] clients;
	delete[] peers;

	printf("Many sockets: ok (%u)\n", (unsigned)(count * 2 + 1));
};

void test() {
	testURL();
	testIP4();
	testAddress();
	testLinkLocal();
	testManySockets();
	printf("Done.\n");
};

int main(int cmdN, char *cmdS[]) {
	try {
		test();
		return 0;
	} catch (const std::exception &e) {
		printf("* Error: %s\n", e.what());
	} catch (...) {
		printf("* Error: Unknown\n");
	};

	return 1;
};
