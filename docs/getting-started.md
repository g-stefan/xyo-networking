# Getting started

## 1. Build and install

The library is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool used by all XYO C++ projects. `xyo-platform`,
`xyo-managed-memory`, `xyo-data-structures`, `xyo-multithreading`,
`xyo-encoding` and `xyo-system` must be installed to the SDK first. From
the repository root:

```bash
fabricare make       # build into output/
fabricare test       # build and run test/test.*.cpp (run make first)
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
```

One project is produced, `xyo-networking` (`"make": "dll-or-lib"`): a DLL /
shared library by default.

## 2. Depend on it from another fabricare project

In the consumer's `fabricare.json`:

```json
{
	"name": "my-tool",
	"make": "exe",
	"sourcePath": "XYO/MyTool",
	"dependency": [
		"xyo-networking"
	]
}
```

`xyo-system` and the layers below come in as transitive dependencies. On
Windows `Networking.Amalgam.cpp` links `ws2_32.lib` with a `#pragma`.

## 3. Include

```cpp
#include <XYO/Networking.hpp>
```

The umbrella header pulls in `<XYO/System.hpp>` (and through it every layer
below) and every public header of this library.

Namespace `XYO::Networking` contains:

```cpp
using namespace XYO::ManagedMemory;
using namespace XYO::DataStructures;
using namespace XYO::Encoding;
using namespace XYO::Multithreading;
using namespace XYO::System;
```

so `using namespace XYO::Networking;` is enough to write `String`,
`IApplication`, `Stream::readLn`, `Thread`, `Socket`, `URL::getQuery`, ...
unqualified.

`Network::` and `URL::` are **namespaces of free functions**, not classes.
The classes are `Socket`, `IP4`, `IP6`, `IPAddress4` and `IPAddress6`.

The metadata namespaces exist in every layer (`Version`, `Copyright`,
`License`): write `XYO::Networking::Version::version()` in full.

## 4. First programs

### A client

Download a page over plain HTTP and print it:

```cpp
#include <XYO/Networking.hpp>

using namespace XYO::Networking;

class Application : public virtual IApplication {
		XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Application);

	public:
		inline Application(){};

		int main(int cmdN, char *cmdS[]);
};

int Application::main(int cmdN, char *cmdS[]) {
	String url = "http://example.com/";
	if (cmdN > 1) {
		url = cmdS[1];
	};

	String host;
	String path;
	if (!(URL::getHostNameAndPort(url, host) && URL::getPathAndFileName(url, path))) {
		printf("* Error: invalid URL %s\n", url.value());
		return 1;
	};

	Socket socket;
	// Resolves the name, port 80 from the "http" scheme
	if (!socket.openClientX(url)) {
		printf("* Error: can not connect to %s\n", host.value());
		return 1;
	};

	String request;
	request << "GET " << path << " HTTP/1.0\r\n";
	request << "Host: " << host << "\r\n";
	request << "Connection: close\r\n\r\n";
	if (Stream::write(socket, request) != request.length()) {
		printf("* Error: send\n");
		return 1;
	};

	// read returns 0 when the server closes the connection
	char buffer[4096];
	size_t ln;
	while ((ln = socket.read(buffer, sizeof(buffer))) > 0) {
		fwrite(buffer, 1, ln, stdout);
	};
	return 0;
};

XYO_APPLICATION_MAIN(Application);
```

### A server

A line echo server on port 7000, one client at a time, that stops when a
client sends `quit`:

```cpp
#include <XYO/Networking.hpp>

using namespace XYO::Networking;

class Application : public virtual IApplication {
		XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Application);

	public:
		inline Application(){};

		int main(int cmdN, char *cmdS[]);
};

int Application::main(int cmdN, char *cmdS[]) {
	Socket server;
	// "[::]" listens on every interface, IPv6 and IPv4
	if (!(server.openServerX("[::]:7000") && server.listen(16))) {
		printf("* Error: can not listen on port 7000\n");
		return 1;
	};

	bool running = true;
	while (running) {
		Socket client;
		if (!server.accept(client)) {
			continue;
		};

		String line;
		// readLn keeps the line end, false when the client closes
		while (Stream::readLn(client, line, 4096)) {
			if (line.trimASCII() == "quit") {
				running = false;
				break;
			};
			if (Stream::write(client, line) != line.length()) {
				break;
			};
		};
		// client closes when it goes out of scope
	};
	return 0;
};

XYO_APPLICATION_MAIN(Application);
```

Try it with `telnet localhost 7000` or `nc localhost 7000`.

`XYO_APPLICATION_MAIN(Application)` comes from `xyo-system`: it initializes
the managed memory registry, then calls `Application::main`.

As a fabricare test project:

```json
{
	"name": "test.02",
	"make": "exe",
	"category": "test",
	"SPDX-License-Identifier": "Unlicense",
	"dependency": [
		"xyo-networking"
	]
}
```

with the source in `test/test.02.cpp`.

## 5. Conventions used by the whole library

- **`bool` means success.** `open*`, `listen`, `accept` and the `URL::`
  getters return `true` on success and `false` on any failure; there are no
  exceptions and no error codes (no `errno` / `WSAGetLastError` is
  exposed). Check the result.
- **Sizes tell the rest.** `read` returns the bytes received, `0` on end of
  stream or error. `write` returns the bytes sent; less than asked means
  the connection failed. `waitToRead` / `waitToWrite` return `1` ready,
  `0` timeout, `-1` error or not open.
- **Timeouts are in microseconds** (`uint32_t`, at most about 71 minutes):
  `1000 * 1000` is one second.
- **Opening closes first.** Every `open*` and `accept(peer)` closes what the
  target socket held before, so a `Socket` can be reused.
- **One thread per object.** Like every XYO object, a `Socket` is used by
  one thread at a time. `Network::isValid()` and the `URL::` functions keep
  no state and may be called from any thread.
- Classes are not copyable or movable
  (`XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE`): the address classes have a
  `copy(other)` method, a `Socket` is passed by reference.

## 6. Building without fabricare

Compile the seven amalgams with your sources:

1. Put `source/` of `xyo-platform`, `xyo-managed-memory`,
   `xyo-data-structures`, `xyo-multithreading`, `xyo-encoding`,
   `xyo-system` and `xyo-networking` on the include path.
2. Provide the configuration headers of the layers that have one (copy
   `Config.Template.hpp` to `Config.hpp` for `xyo-platform`,
   `xyo-managed-memory` and `xyo-system`, see their documentation).
   `xyo-networking` has no configuration header.
3. Compile `Platform.Amalgam.cpp`, `ManagedMemory.Amalgam.cpp`,
   `DataStructures.Amalgam.cpp`, `Multithreading.Amalgam.cpp`,
   `Encoding.Amalgam.cpp`, `System.Amalgam.cpp` and
   `Networking.Amalgam.cpp` together with your sources, and define
   `XYO_PLATFORM_LIBRARY`, `XYO_MANAGEDMEMORY_LIBRARY`,
   `XYO_DATASTRUCTURES_LIBRARY`, `XYO_MULTITHREADING_LIBRARY`,
   `XYO_ENCODING_LIBRARY`, `XYO_SYSTEM_LIBRARY` and
   `XYO_NETWORKING_LIBRARY` everywhere (plain static linking).
4. Link `pthread` on Linux; `ws2_32` and `user32` on Windows (pulled in by
   a `#pragma` with MSVC).

Example on Linux, with the repositories side by side:

```bash
g++ -std=c++17 \
    -Ixyo-platform/source -Ixyo-managed-memory/source \
    -Ixyo-data-structures/source -Ixyo-multithreading/source \
    -Ixyo-encoding/source -Ixyo-system/source -Ixyo-networking/source \
    -DXYO_PLATFORM_LIBRARY -DXYO_MANAGEDMEMORY_LIBRARY \
    -DXYO_DATASTRUCTURES_LIBRARY -DXYO_MULTITHREADING_LIBRARY \
    -DXYO_ENCODING_LIBRARY -DXYO_SYSTEM_LIBRARY -DXYO_NETWORKING_LIBRARY \
    xyo-platform/source/XYO/Platform.Amalgam.cpp \
    xyo-managed-memory/source/XYO/ManagedMemory.Amalgam.cpp \
    xyo-data-structures/source/XYO/DataStructures.Amalgam.cpp \
    xyo-multithreading/source/XYO/Multithreading.Amalgam.cpp \
    xyo-encoding/source/XYO/Encoding.Amalgam.cpp \
    xyo-system/source/XYO/System.Amalgam.cpp \
    xyo-networking/source/XYO/Networking.Amalgam.cpp \
    main.cpp -o main -pthread
```

On Windows with MSVC, also define `XYO_PLATFORM_COMPILE_STATIC`:

```bat
cl /EHsc /std:c++17 ^
   /Ixyo-platform\source /Ixyo-managed-memory\source ^
   /Ixyo-data-structures\source /Ixyo-multithreading\source ^
   /Ixyo-encoding\source /Ixyo-system\source /Ixyo-networking\source ^
   /DXYO_PLATFORM_COMPILE_STATIC /DXYO_PLATFORM_LIBRARY /DXYO_MANAGEDMEMORY_LIBRARY ^
   /DXYO_DATASTRUCTURES_LIBRARY /DXYO_MULTITHREADING_LIBRARY ^
   /DXYO_ENCODING_LIBRARY /DXYO_SYSTEM_LIBRARY /DXYO_NETWORKING_LIBRARY ^
   xyo-platform\source\XYO\Platform.Amalgam.cpp ^
   xyo-managed-memory\source\XYO\ManagedMemory.Amalgam.cpp ^
   xyo-data-structures\source\XYO\DataStructures.Amalgam.cpp ^
   xyo-multithreading\source\XYO\Multithreading.Amalgam.cpp ^
   xyo-encoding\source\XYO\Encoding.Amalgam.cpp ^
   xyo-system\source\XYO\System.Amalgam.cpp ^
   xyo-networking\source\XYO\Networking.Amalgam.cpp ^
   main.cpp
```
