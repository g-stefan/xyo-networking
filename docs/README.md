# XYO Networking — Documentation

`xyo-networking` is the network layer of the XYO C++ library stack. It sits
on top of `xyo-system` and gives XYO programs **one portable API for Windows
and Linux to TCP sockets**, plus the small pieces that go with them:

- **TCP sockets.** `Socket` is a blocking TCP stream, client or server,
  IPv4 or IPv6. It implements the `IRead` / `IWrite` interfaces of
  `xyo-data-structures`, so every stream helper of `xyo-system`
  (`Stream::readLn`, `Stream::write`, ...) works on a connection.
  `waitToRead` / `waitToWrite` wait with a timeout, so a thread can poll a
  stop flag instead of blocking forever.
- **Open by name.** `openClientX("example.com:80")`,
  `openServerX("[::]:8080")`, or a whole URL
  (`openClientX("http://user:pw@host/path")`): the host is resolved with
  `getaddrinfo`, every resolved address is tried in order, the port comes
  from the text or from the scheme (`http`, `https`, `ftp`, `ssh`,
  `telnet`).
- **IP addresses.** `IP4` / `IP6` (raw bytes, network order, same network
  test with a mask), `IPAddress4` / `IPAddress6` (address + port, and the
  IPv6 `scopeId` needed for link-local `fe80::` addresses).
- **URL helpers.** `URL::` splits a URL into scheme, user info, host and
  port, path, query, and percent-encodes / decodes a component.
- **Socket subsystem setup.** `Network::isValid()` starts Winsock once per
  process on Windows and raises the open file limit on Linux; every `open*`
  calls it for you.

```
quantum-script--socket, proxy-forward, applications ...
xyo-networking        <-- this library
xyo-system            (IApplication, Shell, File, Stream::readLn, Thread via xyo-multithreading)
xyo-encoding          (String, THex, Base64)
xyo-multithreading    (Thread, Worker, Semaphore)
xyo-data-structures   (containers, IRead / IWrite / ISeek)
xyo-managed-memory    (Object, TPointer, per-thread pools)
xyo-platform          (macros, TAtomic, CriticalSection)
```

## Why it exists

BSD sockets and Winsock look alike but differ in every detail that matters
for a correct program: how the subsystem is started, what a socket handle
is, how many sockets `select` can watch, what happens when the peer closes
the connection, how signals interrupt a call, whether an IPv6 server also
accepts IPv4. `xyo-networking` hides those differences behind one small API
that works with the XYO types (`String`, `IRead`, `IWrite`) and fixes the
cases where a naive port goes wrong:

| Problem | What `xyo-networking` does |
|---------|----------------------------|
| Winsock must be started before the first socket and stopped at the end | `Network::isValid()` calls `WSAStartup` once per process (a `TSingletonProcess`), `WSACleanup` at exit; every `open*` calls it |
| Linux default limit of 1024 open files | the first `Network::isValid()` raises the soft `RLIMIT_NOFILE` to the hard limit |
| `select` cannot watch a socket with a value above 1024 on Linux | `waitToRead` / `waitToWrite` use `ppoll` on Linux (Windows `select` has no such limit) |
| Writing to a connection closed by the peer kills the process with `SIGPIPE` (Linux) | `write` uses `MSG_NOSIGNAL`: it fails and returns less than asked |
| A signal interrupts `recv` / `send` / `accept` / `connect` with `EINTR` (Linux) | all of them are retried; an interrupted `connect` is waited for |
| Sockets leak into child processes started with `Shell::execute` | sockets are created not inheritable (`SOCK_CLOEXEC`, `WSA_FLAG_NO_HANDLE_INHERIT`) |
| An IPv6 server accepts IPv4 on Linux but not on Windows | `IPV6_V6ONLY` is turned off on both: a server on `[::]` accepts IPv4 and IPv6 |
| `SO_REUSEADDR` on Windows lets another process steal the port | set on Linux only (fast restart after `TIME_WAIT`), never on Windows |
| `send` may write less than asked | `write` loops until everything is sent or an error happens |
| Link-local IPv6 (`fe80::`) needs the interface index | `IPAddress6::scopeId`, and `[fe80::1%3]:port` in `openClientX` |

What it does **not** do: no UDP, no non-blocking or asynchronous I/O, no
TLS, no connect timeout (the operating system default applies), no text to
`IP4` / `IP6` parser (use `openClientX` / `openServerX` with text, or fill
the bytes).

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| Connect to a server | `Socket s; s.openClientX("host:port")` | name or IP, `[ip6]:port`, or a URL |
| Listen for connections | `server.openServerX("0.0.0.0:port") && server.listen(n)` | `"[::]:port"` = IPv6 + IPv4 |
| Accept one connection | `Socket peer; server.accept(peer)` | blocks; `server.waitToRead(us)` first to poll |
| Send bytes / text | `s.write(data, size)`, `Stream::write(s, text)` | returns what was sent; less = error |
| Receive bytes | `s.read(buffer, size)` | blocks; `0` = peer closed or error |
| Receive one line | `Stream::readLn(s, line, maxSize)` | keeps the `"\r\n"`; one byte per call, never reads past the line |
| Wait with a timeout | `s.waitToRead(us)`, `s.waitToWrite(us)` | `1` ready, `0` timeout, `-1` error |
| Who is the peer | `peer.getIPAddress4(a)` / `getIPAddress6(a)` | IPv4 client on an IPv6 server = `::ffff:a.b.c.d` |
| Connect by numbers | `IPAddress4 a; a.ip.ip[0] = 127; ...; a.port = 80; s.openClient(a)` | bytes in network order |
| Link-local IPv6 | `IPAddress6::scopeId` or `"[fe80::1%3]:port"` | required on Linux |
| Same subnet? | `ip.isInTheSameNetwork(other, mask)` | `IP4` and `IP6` |
| Parts of a URL | `URL::getHostNameAndPort(url, out)`, `getPathAndFileName`, `getQuery`, ... | `false` if the URL has no `scheme://` |
| Percent-encode | `URL::encodeComponent(text)`, `URL::decodeComponent(text)` | like JavaScript `encodeURIComponent` |

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build it, depend on it, include and namespaces, a first client and server, conventions, building without fabricare |
| [Sockets](sockets.md) | `Socket`: address text, client, server, read / write / wait, streams, closing, threads, platform behavior |
| [IP addresses](addresses.md) | `IP4`, `IP6`, `IPAddress4`, `IPAddress6`, byte order, IPv4-mapped addresses, `scopeId` and link-local, masks |
| [URL](url.md) | `URL::` parsing and percent encoding |
| [API reference](reference.md) | Every public symbol on one page |

The types used everywhere here come from the layers below: `String` from
`xyo-encoding`, the `IRead` / `IWrite` interfaces from
`xyo-data-structures`, `IApplication`, `Stream::` and `Thread` from
`xyo-system` / `xyo-multithreading`. Each repository has its own `docs/`.

## Source map

```
source/XYO/Networking.hpp                    umbrella header, include this
source/XYO/Networking.Amalgam.cpp            compiled part of the library in one translation unit (links ws2_32 on Windows)
source/XYO/Networking/
    Dependency.hpp                           xyo-system, export macro, namespaces
    Network[.hpp, -OS-*.cpp]                 Network::isValid, Winsock start / Linux file limit
    Socket[.hpp, .cpp, -OS-*.cpp]            TCP socket; Socket.cpp has the address text parser
    IP4 / IP6[.hpp, .cpp]                    raw IPv4 / IPv6 address
    IPAddress4 / IPAddress6[.hpp, .cpp]      address + port (+ scopeId)
    URL[.hpp, .cpp]                          URL parts, percent encoding
    Copyright / License / Version            library metadata
test/test.01.cpp                             URL, IP4, address text, IPv6, link-local, more than 1024 sockets, SIGPIPE
```

## AI assistant skill

A Claude Code skill describing how to use this library lives in
[`.claude/skills/xyo-networking/`](../.claude/skills/xyo-networking/SKILL.md).
It is picked up automatically inside this repository; copy the folder to
`~/.claude/skills/` to have it available in the projects that depend on
`xyo-networking`.
