# Networking

C++ library
- One portable API for Windows and Linux to TCP sockets, for the XYO C++
libraries and applications.
- `Socket`: blocking TCP client and server, IPv4 and IPv6 (dual stack servers,
link-local `scopeId`), an `IRead` / `IWrite` stream, `waitToRead` / `waitToWrite`
with timeouts, open by text (`"host:port"`, `"[ip6]:port"`, `"http://host/path"`).
- `IP4`, `IP6`, `IPAddress4`, `IPAddress6`: binary addresses, same network test.
- `URL::`: scheme, user info, host and port, path, query, percent encoding.
- `Network::isValid`: Winsock start on Windows, open file limit on Linux.

Built on `xyo-system`; used by `quantum-script--socket`, `proxy-forward` and
other XYO C++ applications.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, depend on it, first client and server, conventions
- [Sockets](docs/sockets.md) - `Socket`: address text, client, server, read / write / wait, streams, threads
- [IP addresses](docs/addresses.md) - `IP4`, `IP6`, `IPAddress4`, `IPAddress6`, byte order, link-local
- [URL](docs/url.md) - `URL::` parsing and percent encoding
- [API reference](docs/reference.md)

A Claude Code skill for this library is in
[.claude/skills/xyo-networking](.claude/skills/xyo-networking/SKILL.md).

## License

Copyright (c) 2016-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
