cmake_minimum_required(VERSION 3.10)
project(DNet)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(BUILD_EXAMPLES "Build examples" ON)

if(WIN32)
    set(DNET_PLATFORM_WINDOWS ON)
elseif(UNIX AND NOT APPLE)
    set(DNET_PLATFORM_LINUX ON)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(DNET_SOURCES
    "include/DNet/DNetDefs.h"
    "include/DNet/Buffer.h"
    "src/Buffer.cpp"
    "include/DNet/Event/Event.h"
    "include/DNet/Event/EventHandler.h"
    "include/DNet/Ip/Address.h"
    "src/Ip/Address.cpp"
    "include/DNet/Ip/Endpoint.h"
    "src/Ip/Endpoint.cpp"
    "include/DNet/Error.h"
    "src/Error.cpp"
    "include/DNet/Event/EventPool.h"
    "src/Event/EventPool.cpp"
    "include/DNet/Event/EventLoop.h"
    "src/Event/EventLoop.cpp"
    "include/DNet/TcpConnection.h"
    "src/TcpConnection.cpp"
    "include/DNet/TcpServer.h"
    "src/TcpServer.cpp"
    "include/DNet/TcpClient.h"
    "src/TcpClient.cpp"
    "include/DNet/UdpSocket.h"
    "src/UdpSocket.cpp"
)

if(DNET_PLATFORM_WINDOWS)
    list(APPEND DNET_SOURCES
        "src/NetInit.h"
        "src/NetInit.cpp"
        "src/ErrorWin.cpp"
        "src/Event/EventLoopWin.cpp"
        "src/TcpConnectionWin.cpp"
        "src/TcpServerWin.cpp"
        "src/TcpClientWin.cpp"
        "src/UdpSocketWin.cpp"
    )
elseif(DNET_PLATFORM_LINUX)
    list(APPEND DNET_SOURCES
        "src/ErrorLinux.cpp"
        "src/Event/EventLoopLinux.cpp"
        "src/TcpConnectionLinux.cpp"
        "src/TcpServerLinux.cpp"
        "src/TcpClientLinux.cpp"
        "src/UdpSocketLinux.cpp"
    )
endif()

add_library(DNet STATIC ${DNET_SOURCES})
target_include_directories(DNet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(DNET_PLATFORM_WINDOWS)
    target_compile_definitions(DNet PUBLIC DNET_WINDOWS)
    target_link_libraries(DNet PUBLIC ws2_32)
elseif(DNET_PLATFORM_LINUX)
    target_compile_definitions(DNet PUBLIC DNET_LINUX)
    target_link_libraries(DNet PUBLIC uring)
endif()

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()