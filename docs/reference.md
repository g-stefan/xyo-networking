# API reference

Every public symbol of `xyo-networking`, all in namespace `XYO::Networking`.
Include `<XYO/Networking.hpp>`.

## Namespaces brought in

`XYO::Networking` does `using namespace` of `XYO::ManagedMemory`,
`XYO::DataStructures`, `XYO::Encoding`, `XYO::Multithreading` and
`XYO::System` (`Dependency.hpp`).

## Macros

| Macro | Meaning |
|-------|---------|
| `XYO_NETWORKING_EXPORT` | `XYO_PLATFORM_LIBRARY_EXPORT` while building the library (`XYO_NETWORKING_INTERNAL`), `XYO_PLATFORM_LIBRARY_IMPORT` for users, empty when `XYO_NETWORKING_LIBRARY` is defined (static linking) |
| `XYO_NETWORKING_HPP`, `XYO_NETWORKING_<NAME>_HPP` | include guards |

## `Network::` — `<XYO/Networking/Network.hpp>`

| Function | Description |
|----------|-------------|
| `bool isValid()` | Start the socket subsystem once per process (process singleton). Windows: `WSAStartup(2.2)`, `WSACleanup` at exit; `false` if Winsock can not start. Linux: raise the soft `RLIMIT_NOFILE` to the hard limit, always `true`. Called by every `Socket::open*`. |

## `Socket` — `<XYO/Networking/Socket.hpp>`

`class Socket : public virtual IRead, public virtual IWrite`, not copyable
or movable. Blocking TCP stream. See [Sockets](sockets.md).

| Member | Description |
|--------|-------------|
| `Socket()` | Not open. Does not start the subsystem. |
| `~Socket()` | `close()`. |
| `operator bool() const` | `true` while open. |
| `bool openClient(IPAddress4 &address)` | Close, then connect to the address. Blocks. |
| `bool openClient(IPAddress6 &address)` | Same, IPv6, uses `scopeId`. |
| `bool openClientX(const String &address)` | Close, parse the [address text](sockets.md#address-text), resolve with `getaddrinfo`, connect to each resolved address in order until one succeeds. |
| `bool openServer(IPAddress4 &address)` | Close, create and bind. Does not listen. Linux: `SO_REUSEADDR`. |
| `bool openServer(IPAddress6 &address)` | Same, IPv6, dual stack (`IPV6_V6ONLY` off: accepts IPv4 too). |
| `bool openServerX(const String &address)` | Close, parse, resolve (`AI_PASSIVE`), bind the first resolved address that works. |
| `bool listen(uint16_t queue)` | Start listening with a backlog of `queue`. `false` if not open. |
| `bool accept(Socket &peer)` | Close `peer`, block until a client connects, open `peer` on the connection with the remote address. `false` if this socket is not open or not bound. |
| `size_t read(void *output, size_t ln)` | Block until data, return up to `ln` bytes. `0` on end of stream or error. `IRead::read`. |
| `size_t write(const void *input, size_t ln)` | Send all `ln` bytes (loops). Returns the count sent, less than `ln` on error. Linux: no `SIGPIPE`. `ln == 0` sends nothing, returns `0`. `IWrite::write`. |
| `int waitToRead(uint32_t microSeconds)` | `1` readable (data, end of stream, or a connection to accept), `0` timeout, `-1` error / not open. |
| `int waitToWrite(uint32_t microSeconds)` | `1` writable, `0` timeout, `-1` error / not open. |
| `void shutdown()` | Stop both directions (`SHUT_RDWR` / `SD_BOTH`). The socket stays open. |
| `void close()` | `shutdown`, close the handle, forget the address. Safe to repeat. |
| `bool isIPAddress4()` | Open and the address is IPv4. |
| `bool isIPAddress6()` | Open and the address is IPv6. |
| `bool getIPAddress4(IPAddress4 &out)` | Copy the address: the server for a client socket, the bound address for a server socket, the remote client for an accepted socket. `false` if not open or IPv6. |
| `bool getIPAddress6(IPAddress6 &out)` | Same for IPv6, with `scopeId`. `false` if not open or IPv4. |

Address text accepted by `openClientX` / `openServerX`: `host:port`,
`ip4:port`, `[ip6]:port`, `[ip6%scope]:port`, or `scheme://[user[:pw]@]host[:port][/path][?query][#fragment]`.
The port may be left out only for the schemes `http` (80), `https` (443),
`ftp` (21), `ssh` (22), `telnet` (23). Port: decimal, at most 65535.

## `IP4` — `<XYO/Networking/IP4.hpp>`

`class IP4 : public Object`, not copyable or movable.

| Member | Description |
|--------|-------------|
| `uint8_t ip[4]` | Address bytes in network order (`ip[0]` is the first number). |
| `IP4()` | `0.0.0.0`. |
| `uint32_t toU32()` | The four bytes as a `uint32_t`, unchanged: network byte order, same as `in_addr.s_addr`. |
| `void fromU32(uint32_t value)` | Inverse of `toU32`. |
| `void copy(IP4 &other)` | Copy the bytes. |
| `bool isInTheSameNetwork(IP4 &ip, IP4 &mask)` | `(this & mask) == (ip & mask)` for every byte. |

## `IP6` — `<XYO/Networking/IP6.hpp>`

`class IP6 : public Object`, not copyable or movable.

| Member | Description |
|--------|-------------|
| `uint8_t ip[16]` | Address bytes in network order. |
| `IP6()` | `::`. |
| `void copy(IP6 &other)` | Copy the bytes. |
| `bool isInTheSameNetwork(IP6 &ip, IP6 &mask)` | `(this & mask) == (ip & mask)` for every byte. |

## `IPAddress4` — `<XYO/Networking/IPAddress4.hpp>`

`class IPAddress4 : public Object`, not copyable or movable.

| Member | Description |
|--------|-------------|
| `IP4 ip` | Address. |
| `uint16_t port` | Port, host number (converted with `htons` / `ntohs` inside). |
| `IPAddress4()` | `0.0.0.0`, port `0`. |
| `void copy(IPAddress4 &other)` | Copy address and port. |

## `IPAddress6` — `<XYO/Networking/IPAddress6.hpp>`

`class IPAddress6 : public Object`, not copyable or movable.

| Member | Description |
|--------|-------------|
| `IP6 ip` | Address. |
| `uint16_t port` | Port, host number. |
| `uint32_t scopeId` | Interface index (`sin6_scope_id`), required for link-local `fe80::/10` addresses, `0` if not used. |
| `IPAddress6()` | `::`, port `0`, `scopeId` `0`. |
| `void copy(IPAddress6 &other)` | Copy address, port and `scopeId`. |

## `URL::` — `<XYO/Networking/URL.hpp>`

Free functions, stateless. The getters return `false` and leave `out`
unchanged when the text has no `scheme://`. Results are not decoded. See
[URL](url.md).

| Function | Description |
|----------|-------------|
| `bool getSchemeName(const String &url, String &out)` | Scheme: a letter, then letters, digits, `+ - .`, followed by `://`. |
| `bool getUsernameAndPassword(const String &url, String &out)` | Authority text before its last `@`. `false` if there is none. |
| `bool getHostNameAndPort(const String &url, String &out)` | Authority without the user info, IPv6 brackets kept. |
| `bool getPathAndFileName(const String &url, String &out)` | From the end of the authority to `?` / `#`; `"/"` when empty. |
| `bool getQuery(const String &url, String &out)` | After `?` up to `#`. `false` if there is no `?` before `#`. |
| `String encodeComponent(const String &value)` | Keep `A-Z a-z 0-9 - _ . ! ~ * ' ( )`, every other byte to `%XX` (upper case). |
| `String decodeComponent(const String &value)` | `%XX` to a byte; invalid or incomplete escapes kept as is; `+` unchanged. |

## Library metadata

| Function | Header |
|----------|--------|
| `const char *Version::version()`, `build()`, `versionWithBuild()`, `datetime()` | `<XYO/Networking/Version.hpp>` |
| `const char *Copyright::copyright()`, `publisher()`, `company()`, `contact()` | `<XYO/Networking/Copyright.hpp>` |
| `std::string License::license()`, `shortLicense()` | `<XYO/Networking/License.hpp>` |

Every XYO layer has these namespaces: write them in full,
`XYO::Networking::Version::version()`.
