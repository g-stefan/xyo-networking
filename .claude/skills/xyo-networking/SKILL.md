---
name: xyo-networking
description: >-
  How to use the xyo-networking C++ library (namespace XYO::Networking), the
  network layer of the XYO C++ stack on top of xyo-system, one API for
  Windows and Linux: the blocking TCP Socket (IRead / IWrite stream) as
  client or server, IPv4 and IPv6 - openClient / openServer with
  IPAddress4 / IPAddress6, openClientX / openServerX with address text
  ("host:port", "[ip6]:port", "[fe80::1%3]:port", "http://host/path"),
  listen, accept, read, write, waitToRead / waitToWrite with a microsecond
  timeout, shutdown, close, isIPAddress4 / isIPAddress6 / getIPAddress4 /
  getIPAddress6; the address classes IP4 (toU32 / fromU32 network byte
  order, isInTheSameNetwork), IP6, IPAddress4, IPAddress6 (port, scopeId for
  link-local); Network::isValid (Winsock start, Linux open file limit); the
  URL:: helpers getSchemeName, getHostNameAndPort, getUsernameAndPassword,
  getPathAndFileName, getQuery, encodeComponent, decodeComponent. Use when
  writing or reviewing code that includes <XYO/Networking.hpp>, depends on
  "xyo-networking" in fabricare.json, uses any of these names, opens TCP
  connections in XYO code, or when working inside the xyo-networking
  repository or programs built on it (quantum-script--socket,
  proxy-forward).
---

# xyo-networking

Network layer of the XYO C++ libraries, on top of `xyo-system` (see the
`xyo-system`, `xyo-encoding`, `xyo-multithreading`, `xyo-data-structures`,
`xyo-managed-memory` and `xyo-platform` skills; their rules all apply).
Purpose: **one portable API for Windows and Linux to TCP sockets**, IPv4 and
IPv6, working with the XYO types (`String`, `IRead` / `IWrite`, so the
`Stream::` helpers of `xyo-system` work on a connection), plus binary IP
address classes and URL parsing / percent encoding.

Full documentation: `docs/` in the xyo-networking repository
(`X:\Storage\XYO\Gitea\CPP\xyo-networking\docs` on this machine): README,
getting-started, **sockets**, addresses, url, reference. Read the matching
page when you need more than this summary. When in doubt read the header in
`source/XYO/Networking/`, the address parser in `Socket.cpp` and the
`-OS-Windows.cpp` / `-OS-Linux.cpp` implementation; `test/test.01.cpp`
shows real usage, `proxy-forward` is a real multi-threaded server.

## Pick a tool

| Need | Use |
|------|-----|
| Connect by name / text | `Socket s; s.openClientX("example.com:80")` (or `"[::1]:80"`, `"http://host/path"`) |
| Connect by numbers | `IPAddress4 a; a.ip.ip[0..3] = ...; a.port = 80; s.openClient(a)` |
| Listen | `server.openServerX("[::]:8080") && server.listen(128)` (`"0.0.0.0:8080"` IPv4 only) |
| Accept | `Socket peer; server.accept(peer)` (blocks) |
| Accept, stoppable | `server.waitToRead(100000)` → `1` then `accept`, `0` check stop flag |
| Send all bytes / a String | `s.write(p, n) == n`, `Stream::write(s, text) == text.length()` |
| Send a line | `Stream::writeLn(s, text)` (appends `"\r\n"`) |
| Receive what arrived | `n = s.read(buf, size)`; `0` = closed or error |
| Receive exactly n bytes | loop on `read` until done, fail on `0` |
| Receive one line (protocol headers) | `Stream::readLn(s, line, maxSize)` (keeps `"\r\n"`) |
| Wait with timeout | `s.waitToRead(us)` / `waitToWrite(us)`: `1` ready, `0` timeout, `-1` error |
| Peer address | `peer.isIPAddress6() ? getIPAddress6(a6) : getIPAddress4(a4)` |
| Link-local IPv6 | `IPAddress6::scopeId = ifIndex` or text `"[fe80::1%3]:port"` |
| Same subnet | `ip.isInTheSameNetwork(network, mask)` (mask as address bytes) |
| IPv4 to / from `in_addr.s_addr` | `IP4::toU32()` / `fromU32()` (network order, no swap) |
| URL parts | `URL::getSchemeName`, `getUsernameAndPassword`, `getHostNameAndPort`, `getPathAndFileName`, `getQuery` |
| Percent-encode a component | `URL::encodeComponent(v)` / `decodeComponent(v)` |

## Hard rules

1. **Entry point and memory**: programs use `XYO_APPLICATION_MAIN(App)` from
   `xyo-system` (registry init first). `Socket` holds OS state: not
   copyable or movable — pass by reference, keep it as a member or local,
   reuse it (every `open*` and `accept(peer)` closes the target first). The
   address classes derive from `Object`, are not copyable: use `copy(other)`.
2. **Check every result**: `bool` for `open*` / `listen` / `accept` /
   `URL::get*`; no exceptions, no error codes, no errno exposed. `write`
   returns the bytes sent — **less than asked = connection broken**.
   `read` returns `0` for **both** peer closed and error.
3. **Blocking only**: `openClient*` (no connect timeout, OS default),
   `accept`, `read` all block. To stay responsive, call
   `waitToRead(microSeconds)` first in a loop that checks a stop flag
   (`1` ready — also on peer close / pending connection, `0` timeout,
   `-1` error / not open). Timeouts are **microseconds** (`uint32_t`).
4. **Server = openServer\* + listen**: `openServer*` only creates and binds.
   Host is required in text: `":8080"` fails; use `"0.0.0.0:port"` or
   `"[::]:port"`. An IPv6 server is **dual stack on both OSes**: IPv4
   clients appear as `::ffff:a.b.c.d` (`isIPAddress6() == true`, IPv4 in
   `ip[12..15]`). `SO_REUSEADDR` is set on Linux only — do not "fix" it on
   Windows (port stealing). Port `0` binds a random port but
   `getIPAddress*` returns `0`: no way to read it; try a port range.
5. **Address text** (`openClientX` / `openServerX`): `host:port`,
   `[ip6]:port`, `[ip6%scope]:port`, or a URL (user info / path / query
   ignored). The port is required unless the scheme is `http` 80,
   `https` 443, `ftp` 21, `ssh` 22, `telnet` 23. Port: digits only,
   ≤ 65535. IPv6 with a port needs brackets. Names go through
   `getaddrinfo(AF_UNSPEC)` and **every result is tried in order**.
   No TLS: `https://` gives a plain TCP connection to 443.
6. **TCP is a byte stream**: one `read` may return part of a peer `write`
   or several joined. Frame messages yourself (length prefix +
   read-exact loop, or lines). `Stream::readLn` reads **one byte per call
   on purpose** so it never consumes bytes after the line (HTTP body) — do
   not replace it with a buffered reader on a socket; `Stream::LineRead`
   needs `ISeek` and does not apply. `Stream::read(s, out, n)` stops at the
   first short read (not an exact read).
7. **Closing**: `close()` = `shutdown()` (both directions) + close + forget
   address; idempotent; the destructor calls it. No half close: end
   requests inside the protocol (`Content-Length`, `Connection: close`,
   terminator line). `operator bool` = open, **not** "peer still
   connected".
8. **Threads**: one owner thread per `Socket` (XYO rule). For concurrent
   clients, `accept(conn.socket)` into a socket the worker owns, then
   `conn.thread.start((ThreadProcedure)serve, &conn)`. Never close or use a
   socket from another thread while its owner is blocked on it; use
   `waitToRead` timeouts to let owners notice a stop flag.
   `Network::isValid()` and `URL::` are thread safe.
9. **Addresses**: `ip[]` bytes are network order as written
   (`192.168.1.2` → `{192,168,1,2}`); `port` is a host number (no
   `htons`). `toU32` / `fromU32` copy bytes unchanged (network order, like
   `s_addr`) — not for arithmetic or printing. **No text parser or
   formatter** for `IP4` / `IP6`: use `openClientX` text, print bytes with
   `printf`. Masks are addresses (`255.255.255.0`), not prefix lengths.
   Link-local `fe80::/10` needs `scopeId` (Linux connect fails without it).
10. **URL::** getters need `scheme://` (letter first; `mailto:` and bare
    `host/path` → `false`, `out` untouched). Results are raw (not decoded).
    `getHostNameAndPort` keeps IPv6 brackets and the port together;
    `getPathAndFileName` gives `"/"` when empty; `getQuery` excludes the
    fragment; no fragment getter. `encodeComponent` = JS
    `encodeURIComponent` per byte (space → `%20`); `decodeComponent` keeps
    invalid escapes and does **not** map `+` to space (do it first for form
    data).
11. **Platform work is already done** — do not add it again: Winsock start /
    cleanup (`Network::isValid`, called by every `open*`), Linux
    `RLIMIT_NOFILE` raise, `ppoll` instead of `select` (sockets above 1024),
    `EINTR` retries, `MSG_NOSIGNAL` (no `SIGPIPE`), non-inheritable sockets
    (`SOCK_CLOEXEC` / `WSA_FLAG_NO_HANDLE_INHERIT`), `write` loop over
    partial sends, `ws2_32.lib` via `#pragma`.

## Minimal patterns

```cpp
#include <XYO/Networking.hpp>

using namespace XYO::Networking;

// client: request / response
static bool httpGet(const String &url, String &response) {
	String host;
	String path;
	if (!(URL::getHostNameAndPort(url, host) && URL::getPathAndFileName(url, path))) {
		return false;
	};
	Socket socket;
	if (!socket.openClientX(url)) { // port 80 from "http"
		return false;
	};
	String request;
	request << "GET " << path << " HTTP/1.0\r\nHost: " << host << "\r\nConnection: close\r\n\r\n";
	if (Stream::write(socket, request) != request.length()) {
		return false;
	};
	char buffer[16384];
	size_t ln;
	response = "";
	while ((ln = socket.read(buffer, sizeof(buffer))) > 0) {
		response.concatenate(buffer, ln);
	};
	return true;
};

// exact read for framed protocols
static bool readExact(Socket &socket, void *output, size_t ln) {
	char *buffer = static_cast<char *>(output);
	size_t done = 0;
	while (done < ln) {
		size_t readLn = socket.read(buffer + done, ln - done);
		if (readLn == 0) {
			return false;
		};
		done += readLn;
	};
	return true;
};

// server: stoppable accept loop + line protocol
static bool serve(TAtomic<bool> &stop) {
	Socket server;
	if (!(server.openServerX("[::]:7000") && server.listen(128))) {
		return false;
	};
	while (!stop.get()) {
		int state = server.waitToRead(100 * 1000); // 100 ms
		if (state == 0) {
			continue;
		};
		if (state < 0) {
			break;
		};
		Socket peer;
		if (!server.accept(peer)) {
			continue;
		};
		String line;
		while (Stream::readLn(peer, line, 8192)) { // line keeps "\r\n"
			if (line.trimASCII() == "quit") {
				break;
			};
			if (Stream::write(peer, line) != line.length()) {
				break;
			};
		};
	};
	return true;
};
```

## Using it

- `#include <XYO/Networking.hpp>` (umbrella). C++17. `XYO::Networking`
  does `using namespace` of `XYO::ManagedMemory`, `XYO::DataStructures`,
  `XYO::Encoding`, `XYO::Multithreading`, `XYO::System`. `Network::` and
  `URL::` are namespaces of free functions; `Version` / `Copyright` /
  `License` exist in every layer: write `XYO::Networking::Version::version()`.
- fabricare consumer: `"dependency": ["xyo-networking"]` (DLL / shared
  library, `dll-or-lib`; there is no `.static` project). Install
  `xyo-platform`, `xyo-managed-memory`, `xyo-data-structures`,
  `xyo-multithreading`, `xyo-encoding`, `xyo-system`, then this library to
  the SDK before building dependents (see the `fabricare` skill).
  `fabricare make`, `fabricare test`, `fabricare install`.
- Without fabricare: compile the seven amalgams (`Platform`,
  `ManagedMemory`, `DataStructures`, `Multithreading`, `Encoding`,
  `System`, `Networking`) with all `-DXYO_<LIB>_LIBRARY` defines (MSVC: also
  `/DXYO_PLATFORM_COMPILE_STATIC`), the `Config.hpp` copies the lower
  layers need, `-pthread` on Linux, `ws2_32` + `user32` on Windows.

## Code style (match the repository)

- Tabs (width 8), `.clang-format` in the repo, CRLF line endings; statements
  and blocks end with `};`.
- camelCase functions, `retV` for return values, trailing `_` on parameters
  that shadow members (`adr_`, `socket_`) and on file-local helpers
  (`socketOpen_`, `socketParseAddress_`); OS state behind a forward declared
  pimpl (`Socket_ *this_`).
- Headers: include guard `XYO_NETWORKING_<NAME>_HPP`, guarded includes
  (`#ifndef XYO_NETWORKING_DEPENDENCY_HPP #include ...`). Add new headers to
  `source/XYO/Networking.hpp`, new `.cpp` files (including `-OS-Windows.cpp`
  / `-OS-Linux.cpp` pairs, each wrapped in `#ifdef XYO_PLATFORM_OS_*`) to
  `source/XYO/Networking.Amalgam.cpp`. OS independent socket code goes in
  `Socket.cpp`. Exported functions use `XYO_NETWORKING_EXPORT`; classes are
  `XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE`.
- Keep Windows and Linux behavior identical (dual stack, no `SIGPIPE`,
  non-inheritable sockets) and comment the *why* of every platform
  difference (`SO_REUSEADDR`, `EINTR`, `select` vs `ppoll`).
- SPDX header: MIT for `source/`, Unlicense for `test/`.
- Tests: `test/test.NN.cpp` with `check(condition, message)` helpers,
  ports picked from a range (`27100..27300`) so parallel runs do not clash,
  skip (print, do not fail) when the machine lacks IPv6 / link-local, plus
  a `"category": "test"` project in `fabricare.json`. Run `fabricare make`
  then `fabricare test`.
