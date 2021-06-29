mtxclient {#mainpage}
=====================

mtxclient is a library for the [Matrix protocol](https://matrix.org/) used
primarily in the [Nheko client](https://nheko.im/nheko-reborn/nheko). This
library is not a full SDK. Rather it is a thin wrapper providing functions and
data structures to access the [Client-Server
API](https://matrix.org/docs/spec/client_server/latest). Most of the semantics
are described there and this library provides similarly named types and function
to invoke those endpoints.

mtxclient is built on top of
[curl](https://curl.se/libcurl/) and
[nlohmann/json](https://github.com/nlohmann/json). Many of the data structures
in this library can be serialized to and deserialized from JSON. Most endpoints
use a callback to inform the caller, if the request completed and what the
response was.

## Building

For build instructions please refer to the [README](README.md).
