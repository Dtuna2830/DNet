<img src="assets/header.png" alt="DNet logo" width="380" />

# DNet

DNet is a lightweight & simple, completion-based C++ networking library for TCP and UDP.

## Features

- Completion-based, asynchronous networking built around event loop.
- Unified support for both stream-based (TCP) and datagram-based (UDP) communication.
- Cross-platform design with native backends for Windows and Linux.
- IPv4/IPv6 endpoint support for local and remote addressing.
- Explicit, structured error reporting for predictable failure handling.

## Quick Start

```cpp
#include <iostream>
#include <DNet/UdpSocket.h>

using namespace DNet;

int main() {
    EventLoop loop;
    Endpoint local = Endpoint::LoopbackV4(1234);
    UdpSocket socket(local, loop);

    socket.onData([&](size_t len, const char* data, const Endpoint&) {
        std::cout << "Received: " << std::string(data, len) << std::endl;
    });

    Error err = socket.bind();
    if (!err.ok()) {
        return -1;
    }

    Error recvErr = socket.startReceiving();
    if (!recvErr.ok()) {
        return -1;
    }

    loop.run();
    return 0;
}
```

## License

DNet is distributed under the terms described in [LICENSE](LICENSE).