# Sockets

`Socket` (`<XYO/Networking/Socket.hpp>`) is a **blocking TCP stream**,
IPv4 or IPv6, used as a client (connect to a server) or as a server (listen
and accept connections). It derives from `IRead` and `IWrite`
(`xyo-data-structures`), so it is a stream like `File` or
`ProcessInteractive`.

```cpp
class Socket : public virtual IRead, public virtual IWrite {
	public:
		Socket();
		~Socket();                                     // close()
		operator bool() const;                         // true while open

		bool openClient(IPAddress4 &);                 // connect
		bool openClient(IPAddress6 &);
		bool openClientX(const String &address);       // resolve text, then connect
		bool openServer(IPAddress4 &);                 // create + bind
		bool openServer(IPAddress6 &);
		bool openServerX(const String &address);       // resolve text, then bind
		bool listen(uint16_t queue);
		bool accept(Socket &peer);

		size_t read(void *output, size_t ln);
		size_t write(const void *input, size_t ln);
		int waitToRead(uint32_t microSeconds);
		int waitToWrite(uint32_t microSeconds);

		void shutdown();
		void close();

		bool isIPAddress4();
		bool isIPAddress6();
		bool getIPAddress4(IPAddress4 &);
		bool getIPAddress6(IPAddress6 &);
};
```

## Address text

`openClientX` and `openServerX` take the address as text. Accepted forms:

| Text | Host | Port |
|------|------|------|
| `127.0.0.1:8080` | `127.0.0.1` | `8080` |
| `example.com:443` | resolved with `getaddrinfo` | `443` |
| `[::1]:8080` | `::1` | `8080` |
| `[fe80::1%3]:8080` | `fe80::1`, interface index `3` (`%eth0` also works on Linux) | `8080` |
| `[::]:8080` | every IPv6 and IPv4 interface (server) | `8080` |
| `0.0.0.0:8080` | every IPv4 interface (server) | `8080` |
| `http://example.com/path?q=1` | `example.com` | `80`, from the scheme |
| `http://user:pw@example.com:8080/` | `example.com` (the user info is skipped) | `8080` |
| `https://[::1]/` | `::1` | `443` |

Rules:

- The port is required unless the text is a URL whose scheme has a known
  port: `http` 80, `https` 443, `ftp` 21, `ssh` 22, `telnet` 23. Any other
  scheme without a port fails. Path, query and fragment are ignored.
- The port must be decimal digits only and at most `65535`; `"80x"`,
  `"99999"` fail.
- The host is required: `":8080"` fails. To listen everywhere use
  `"0.0.0.0:port"` (IPv4) or `"[::]:port"` (IPv6 + IPv4).
- An IPv6 address with a port needs brackets. Without brackets
  (`"::1"`) the whole text is the host and there is no port, so it works
  only behind a scheme with a default port.
- A name is resolved with `getaddrinfo` (`AF_UNSPEC`, so IPv4 and IPv6).
  **Every resolved address is tried in order** until one connects (client)
  or binds (server): `"localhost:8080"` reaches a server listening on
  `127.0.0.1` even when `localhost` resolves to `::1` first.
- The resolved `sockaddr` is used as is, so the IPv6 scope id written as
  `%index` is kept.

`openClient` / `openServer` take an `IPAddress4` / `IPAddress6` instead (see
[IP addresses](addresses.md)): no name resolution, no text parsing.

## Client

```cpp
Socket socket;
if (!socket.openClientX("example.com:80")) {
	// name not resolved, refused, unreachable, ...
	return false;
};
// connected, read and write
```

`openClient*` creates the socket and **connects, blocking** until the
connection is made or fails. There is no connect timeout: the operating
system default applies (from a few seconds for "refused" to minutes for an
unreachable host).

## Server

```cpp
Socket server;
if (!(server.openServerX("[::]:8080") && server.listen(128))) {
	// port in use, no permission (ports below 1024 on Linux), ...
	return false;
};

for (;;) {
	Socket peer;
	if (!server.accept(peer)) {
		continue;
	};
	IPAddress6 from;
	if (peer.getIPAddress6(from)) { /* ... */ };
	// serve peer, or hand it to a thread (see below)
};
```

- `openServer*` creates the socket and **binds** it; it does not listen.
  Call `listen(queue)` next; `queue` is the backlog of connections waiting
  for `accept`.
- `accept(peer)` **blocks** until a client connects, closes whatever `peer`
  held, and opens `peer` on the new connection. It fails if the server is
  not open.
- To serve without blocking forever (so a stop flag can be checked), wait
  first: on a listening socket `waitToRead` returns `1` when a connection
  is ready to accept.

```cpp
while (!stop) {
	int state = server.waitToRead(100 * 1000); // 100 ms
	if (state == 0) {
		continue; // timeout, check stop again
	};
	if (state < 0) {
		break;    // server socket error
	};
	Socket peer;
	if (server.accept(peer)) {
		// ...
	};
};
```

Server socket options, set for you:

- An IPv6 server is **dual stack** (`IPV6_V6ONLY` off) on both Windows and
  Linux: `"[::]:8080"` accepts IPv4 clients too, seen as IPv4-mapped IPv6
  addresses `::ffff:a.b.c.d`.
- `SO_REUSEADDR` is set **on Linux only**, so a restarted server can bind
  again while old connections are in `TIME_WAIT`. It is not set on Windows,
  where it would let another process bind the same port and take the
  connections.
- Listening on port `0` lets the system choose a port, but
  `getIPAddress4/6` returns the address as given (port `0`): the library has
  no call to read the chosen port. To find a free port, try a range, as
  `test/test.01.cpp` does.

## Reading and writing

```cpp
size_t read(void *output, size_t ln);
size_t write(const void *input, size_t ln);
```

- `read` **blocks until at least one byte arrives**, then returns what is
  there, at most `ln`. It returns `0` when the peer closed the connection
  **or** on error (the two are not told apart). A read may return fewer
  bytes than a single `write` of the peer sent, or join several writes:
  TCP is a byte stream, not messages.
- `write` sends everything: it loops until all `ln` bytes are accepted by
  the system or an error happens, and returns the count sent. **Less than
  `ln` means the connection is broken.** On Linux a write to a connection
  closed by the peer fails instead of raising `SIGPIPE`.
- `write(data, 0)` sends nothing and returns `0`.

Reading an exact number of bytes (a length-prefixed message, a body of
`Content-Length` bytes) needs a loop:

```cpp
static bool readExact(Socket &socket, void *output, size_t ln) {
	char *buffer = static_cast<char *>(output);
	size_t done = 0;
	while (done < ln) {
		size_t readLn = socket.read(buffer + done, ln - done);
		if (readLn == 0) {
			return false; // closed before the end
		};
		done += readLn;
	};
	return true;
};
```

## Waiting with a timeout

```cpp
int waitToRead(uint32_t microSeconds);
int waitToWrite(uint32_t microSeconds);
```

| Return | Meaning |
|--------|---------|
| `1` | ready: `read` / `accept` will not block (`waitToRead`), or there is room to send (`waitToWrite`) |
| `0` | timeout |
| `-1` | error, or the socket is not open |

`waitToRead` returns `1` also when the peer closed the connection: the
`read` that follows returns `0`. `microSeconds = 0` only checks, without
waiting. The usual loop that can be stopped:

```cpp
char buffer[16384];
while (!stop) {
	int state = socket.waitToRead(100 * 1000);
	if (state == 0) {
		continue;          // nothing yet
	};
	if (state < 0) {
		break;             // error
	};
	size_t ln = socket.read(buffer, sizeof(buffer));
	if (ln == 0) {
		break;             // peer closed
	};
	// use buffer[0 .. ln)
};
```

On Linux the wait uses `ppoll` (no limit on the socket number, unlike
`select` and its 1024 `FD_SETSIZE`) and restarts after a signal with the
remaining time; on Windows it uses `select`, which has no such limit there.

## Streams and lines

`Socket` is an `IRead` and an `IWrite`, so the `Stream::` helpers of
`xyo-system` work on it:

```cpp
String line;
while (Stream::readLn(socket, line, 8192)) {   // false at end of stream
	String text = line.trimASCII();           // line keeps its "\r\n" / "\n"
	// ...
};

Stream::write(socket, String("HELLO\r\n"));    // returns bytes sent
Stream::writeLn(socket, "HELLO");              // appends "\r\n"
```

- `Stream::readLn` reads **one byte per call on purpose**: it never takes
  bytes after the line, so after reading the header lines of a protocol
  (HTTP, SMTP, ...) the body is still in the socket for the next `read`.
  A line longer than the size limit comes back in pieces.
- `Stream::read(socket, out, size)` stops at the first short read, so on a
  socket it returns **what has arrived**, not necessarily `size` bytes.
- `Socket` is not an `ISeek`: `Stream::LineRead` does not apply.

## Closing

```cpp
void shutdown();   // stop sending and receiving, the socket stays open
void close();      // shutdown + close + forget the address
```

- `close()` is safe to call more than once; the destructor calls it. Each
  `open*` and `accept(peer)` calls it on the target first, so one `Socket`
  can be reused for many connections.
- `shutdown()` stops **both directions** (`SHUT_RDWR` / `SD_BOTH`); the peer
  sees the end of the stream. There is no half close (stop only sending and
  keep reading the answer): mark the end of a request inside the protocol
  instead (`Content-Length`, `Connection: close`, a terminator line, ...).
- `operator bool()` is `true` between a successful `open*` / `accept` and
  `close`. It does not tell whether the peer is still there: a closed
  connection shows up as `read` returning `0` or `write` returning less.

## Peer address

```cpp
bool isIPAddress4();
bool isIPAddress6();
bool getIPAddress4(IPAddress4 &);
bool getIPAddress6(IPAddress6 &);
```

The socket remembers one address:

| Socket | Address |
|--------|---------|
| client (`openClient*`) | the server it connected to |
| server (`openServer*`) | the address it is bound to, as given |
| accepted (`accept(peer)`) | the remote client |

`getIPAddress4` fails on an IPv6 socket and `getIPAddress6` on an IPv4 one:
check `isIPAddress4()` / `isIPAddress6()` first. All four return `false`
when the socket is not open. The local address of a client connection is
not available.

```cpp
if (peer.isIPAddress4()) {
	IPAddress4 address;
	peer.getIPAddress4(address);
	printf("%u.%u.%u.%u:%u\n", address.ip.ip[0], address.ip.ip[1],
	       address.ip.ip[2], address.ip.ip[3], address.port);
};
```

## Threads

A `Socket` belongs to one thread at a time (the XYO rule). A server that
serves several clients at once accepts in one thread and hands each
connection to a worker thread that owns it from then on. `accept(peer)`
into a `Socket` that the worker thread owns, then start the thread:

```cpp
class Connection {
	public:
		Socket socket;
		Thread thread;
};

static void serve(Connection *connection) {
	String line;
	while (Stream::readLn(connection->socket, line, 8192)) {
		if (Stream::write(connection->socket, line) != line.length()) {
			break;
		};
	};
	connection->socket.close();
};

// in the accept loop, with a free Connection c:
if (server.accept(c.socket)) {
	c.thread.start((ThreadProcedure)serve, &c);
};
```

Use `waitToRead` with a timeout in every loop that must stop on request
(server shutdown, idle timeout): a thread blocked in `read` or `accept`
does not wake up by itself. Do not close a socket from another thread while
its owner is blocked on it.

`Network::isValid()` is thread safe (a process singleton) and is called by
every `open*`.

## Platform behavior

| | Windows | Linux |
|---|---|---|
| Subsystem start | `WSAStartup(2.2)` on the first `open*`, `WSACleanup` at process end | nothing to start; the soft `RLIMIT_NOFILE` is raised to the hard limit (capped by `fs.nr_open`) |
| Wait | `select` (no 64 / 1024 limit on Windows) | `ppoll`, restarted after `EINTR` with the remaining time |
| Signals | — | `recv`, `send`, `accept`, `connect` retried after `EINTR`; `MSG_NOSIGNAL` instead of `SIGPIPE` |
| Inheritance by child processes | `WSA_FLAG_NO_HANDLE_INHERIT`, accepted sockets marked not inheritable | `SOCK_CLOEXEC`, `accept4(..., SOCK_CLOEXEC)` |
| `SO_REUSEADDR` on servers | not set (would allow port stealing) | set |
| IPv6 server accepts IPv4 | yes (`IPV6_V6ONLY = 0`) | yes (`IPV6_V6ONLY = 0`) |
| Link-local IPv6 without `scopeId` | depends on the system: always set it | connect fails |
| One `read` / `write` call | at most `INT_MAX` bytes per system call (`write` loops) | any size |
| Library | `ws2_32.lib` (`#pragma` in the amalgam) | libc |
