# URL

`URL::` (`<XYO/Networking/URL.hpp>`) is a namespace of free functions that
split a URL into its parts and percent-encode / decode a component. They
keep no state and may be called from any thread. `Socket::openClientX` /
`openServerX` use them to read the host and port of a URL.

```
  http://user:pw@example.com:8080/docs/index.html?lang=en&q=a%20b#top
  \__/   \_____/ \______________/\______________/ \____________/ \_/
 scheme user info  host and port  path and file       query    fragment
```

## Parts

```cpp
bool URL::getSchemeName(const String &url, String &out);
bool URL::getUsernameAndPassword(const String &url, String &out);
bool URL::getHostNameAndPort(const String &url, String &out);
bool URL::getPathAndFileName(const String &url, String &out);
bool URL::getQuery(const String &url, String &out);
```

Every function returns `false` (and leaves `out` alone) when the text is
not a URL with a scheme: the scheme is a letter followed by letters, digits,
`+`, `-` or `.`, then **`://`**. So `"example.com/a"`, `"mailto:x@y"` and
`"1http://x"` are not URLs here, and a `://` inside the query
(`"host/p?r=http://x"`) is not mistaken for a scheme.

The parts come back **as written**: nothing is decoded, lower-cased or
validated.

| Function | Returns | For `http://user:pw@example.com:8080/docs/index.html?lang=en#top` |
|----------|---------|------------------------------|
| `getSchemeName` | text before `://` | `http` |
| `getUsernameAndPassword` | user info, before the last `@` of the authority; `false` if there is no `@` | `user:pw` |
| `getHostNameAndPort` | authority without the user info | `example.com:8080` |
| `getPathAndFileName` | from the end of the authority to `?` or `#`; `"/"` when empty | `/docs/index.html` |
| `getQuery` | after `?`, up to `#`; `false` if there is no `?` before `#` | `lang=en` |

Details:

- The authority (user info + host + port) is the text after `://` up to the
  first `/`, `?` or `#`. An `@` in the path or query is not user info.
- `getHostNameAndPort` keeps the brackets of an IPv6 host:
  `"http://[::1]:80/"` gives `"[::1]:80"`. It does not split the port: the
  port is after the last `:` that follows the closing `]` (if any).
- `getUsernameAndPassword` returns `user:pw` in one string: split it at the
  first `:` (`split2`) and decode both with `decodeComponent`.
- There is no function for the fragment (the text after `#`).

```cpp
String url = "https://api.example.com/v1/items?limit=10&name=caf%C3%A9";
String scheme;
String host;
String path;
String query;
URL::getSchemeName(url, scheme);       // "https"
URL::getHostNameAndPort(url, host);    // "api.example.com"
URL::getPathAndFileName(url, path);    // "/v1/items"
if (URL::getQuery(url, query)) {       // "limit=10&name=caf%C3%A9"
	// split at '&' and '=', then URL::decodeComponent each name and value
};
```

## Percent encoding

```cpp
String URL::encodeComponent(const String &value);
String URL::decodeComponent(const String &value);
```

`encodeComponent` works like JavaScript `encodeURIComponent`: the letters,
digits and `- _ . ! ~ * ' ( )` stay, **every other byte** becomes `%XX`
(upper case hex). A UTF-8 character is encoded byte by byte (`é` is
`%C3%A9`); a space is `%20`, not `+`. Use it for one query value or one
path segment, never for a whole URL (it encodes `/`, `?`, `&`, `=`).

`decodeComponent` turns every `%XX` (either case of hex) back into its byte.
An invalid or incomplete escape is kept as is: `"100%"`, `"%4"` and
`"%zz"` decode to themselves. It does **not** turn `+` into a space: for an
HTML form body (`application/x-www-form-urlencoded`) replace `+` with a
space first.

```cpp
String query = String("q=") + URL::encodeComponent("a b&c");
// "q=a%20b%26c"

String value = URL::decodeComponent("a%20b%26c");
// "a b&c"

String form = String("first+name").replace("+", " ");
String name = URL::decodeComponent(form);
// "first name"
```

## Opening a URL

A URL can be passed directly to `Socket::openClientX`: the user info, path,
query and fragment are ignored, and when there is no port the scheme gives
one (`http` 80, `https` 443, `ftp` 21, `ssh` 22, `telnet` 23). See
[Sockets](sockets.md#address-text). The library has no TLS: a socket opened
on an `https` URL is a plain TCP connection to port 443.
