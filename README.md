<img src="assets/header.png" alt="DNet logo" width="380" />

# DNet

Lightweight completion-based C++ networking library for TCP and UDP.

## Features

- Asynchronous, completion-based API built around an event loop
- Unified TCP and UDP support via `AsyncTcpSocket` and `AsyncUdpSocket`
- Native backends for Windows (IOCP) and Linux (io_uring)
- IPv4/IPv6 support
- Explicit error reporting

## Quick Start

```cpp
#include <iostream>
#include <DNet/AsyncUdpSocket.h>
 
using namespace DNet;
 
int main()
{
    EventLoop loop;
    AsyncUdpSocket socket(loop);
 
    socket.open(AddressType::IPv4);
    socket.bind(Endpoint::LoopbackV4(1234));
 
    std::function<void()> recv = [&]()
    {
        socket.asyncRecv([&](const Error &err, const char *data, size_t len, const Endpoint &)
        {
            if (err.ok())
                std::cout << std::string(data, len) << std::endl;
            recv();
        });
    };
    recv();
 
    loop.run();
}
```

## License

DNet is distributed under the terms described in [LICENSE](LICENSE).