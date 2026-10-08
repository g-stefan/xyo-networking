# IP addresses

Four small classes hold binary addresses. They derive from `Object`
(`xyo-managed-memory`) and are not copyable or movable: use `copy(other)`.
All members are public.

| Class | Members | Header |
|-------|---------|--------|
| `IP4` | `uint8_t ip[4]` | `<XYO/Networking/IP4.hpp>` |
| `IP6` | `uint8_t ip[16]` | `<XYO/Networking/IP6.hpp>` |
| `IPAddress4` | `IP4 ip; uint16_t port;` | `<XYO/Networking/IPAddress4.hpp>` |
| `IPAddress6` | `IP6 ip; uint16_t port; uint32_t scopeId;` | `<XYO/Networking/IPAddress6.hpp>` |

The constructors set everything to zero: `0.0.0.0`, `::`, port `0`,
`scopeId` `0`.

## Byte order

`ip[]` holds the address **in network order, as written**: `192.168.1.2`
is `ip[0] = 192, ip[1] = 168, ip[2] = 1, ip[3] = 2`; `::1` is fifteen zero
bytes then `ip[15] = 1`. The `port` is a plain host number (`8080` is
`8080`); the library converts it with `htons` / `ntohs`.

`IP4::toU32()` / `fromU32(value)` copy the four bytes to / from a
`uint32_t` **unchanged**, so the value is in network byte order, the same as
`in_addr.s_addr`. On a little-endian machine `127.0.0.1` is `0x0100007F`.
Use it to exchange an address with a system API, not to do arithmetic or to
print it.

```cpp
IP4 ip;
ip.ip[0] = 192;
ip.ip[1] = 168;
ip.ip[2] = 1;
ip.ip[3] = 2;

struct in_addr inAddr;
inAddr.s_addr = ip.toU32();   // ready for a system call

IP4 other;
other.fromU32(inAddr.s_addr); // 192.168.1.2 again
```

There is **no text parser or formatter** for `IP4` / `IP6`. To use an
address written as text, pass the text to `Socket::openClientX` /
`openServerX`. To print one, format the bytes:

```cpp
printf("%u.%u.%u.%u", ip.ip[0], ip.ip[1], ip.ip[2], ip.ip[3]);
```

## Connecting by numbers

```cpp
IPAddress4 address;
address.ip.ip[0] = 127;
address.ip.ip[1] = 0;
address.ip.ip[2] = 0;
address.ip.ip[3] = 1;
address.port = 8080;

Socket socket;
if (socket.openClient(address)) {
	// ...
};

IPAddress6 any;              // "::", port 0, scopeId 0
any.port = 8080;
Socket server;
if (server.openServer(any) && server.listen(16)) {
	// every interface, IPv6 and IPv4 (dual stack)
};
```

## IPv4 clients on an IPv6 server

A server opened on an IPv6 address is dual stack (see
[Sockets](sockets.md)). An IPv4 client of such a server is reported as an
**IPv4-mapped IPv6 address**, `::ffff:a.b.c.d`: `peer.isIPAddress6()` is
`true`, the IPv4 address is in `ip[12..15]`.

```cpp
static bool isIPv4Mapped(const IP6 &ip6) {
	for (int k = 0; k < 10; ++k) {
		if (ip6.ip[k] != 0) {
			return false;
		};
	};
	return (ip6.ip[10] == 0xFF) && (ip6.ip[11] == 0xFF);
};

static void toIP4(const IP6 &ip6, IP4 &ip4) {
	memcpy(&ip4.ip[0], &ip6.ip[12], 4);
};
```

## Link-local IPv6 and `scopeId`

A link-local address (`fe80::/10`) exists on every interface, so it means
nothing without the interface it belongs to. `IPAddress6::scopeId` is the
**interface index** (`sin6_scope_id`), `0` when not used.

- To connect or listen on a link-local address, set `scopeId`. On Linux a
  connect without it fails.
- In text, write the index after `%` inside the brackets:
  `"[fe80::1%3]:8080"`. On Linux the interface name works too:
  `"[fe80::1%eth0]:8080"`.
- An accepted socket and a client socket report the `scopeId` of the
  connection in `getIPAddress6`.

The interface indexes come from the system (`ip link` / `ifaddrs` on Linux,
`netsh interface ipv6 show interfaces` / `GetAdaptersAddresses` on Windows);
`test/test.01.cpp` shows how to find a link-local address of the machine.

## Same network

```cpp
bool IP4::isInTheSameNetwork(IP4 &ip, IP4 &mask);
bool IP6::isInTheSameNetwork(IP6 &ip, IP6 &mask);
```

`true` when both addresses are equal under the mask, byte by byte
(`(a[k] & mask[k]) == (b[k] & mask[k])`). The mask is an address, not a
prefix length: `/24` is `255.255.255.0`.

```cpp
IP4 network;    // 192.168.1.0
network.ip[0] = 192;
network.ip[1] = 168;
network.ip[2] = 1;

IP4 mask;       // 255.255.255.0
mask.ip[0] = 255;
mask.ip[1] = 255;
mask.ip[2] = 255;

IPAddress4 peerAddress;
if (peer.getIPAddress4(peerAddress) && peerAddress.ip.isInTheSameNetwork(network, mask)) {
	// client from the local network
};
```

## Copying

```cpp
IPAddress4 a;
IPAddress4 b;
b.copy(a);        // ip and port
IPAddress6 c;
IPAddress6 d;
d.copy(c);        // ip, port and scopeId
```

`IP4::copy` / `IP6::copy` copy the bytes only.
