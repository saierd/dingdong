#include "udp.h"

#include <cstring>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

UdpSocket::UdpSocket() {
    handle = socket(AF_INET, SOCK_DGRAM, 0);
}

UdpSocket::~UdpSocket() {
    if (handle) close(handle);
}

UdpSocket::UdpSocket(UdpSocket&& other) noexcept {
    handle = other.handle;
    other.handle = 0;
}

UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
    handle = other.handle;
    other.handle = 0;
    return *this;
}

void UdpSocket::bind(int port) {
    struct sockaddr_in addr;

    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<decltype(addr.sin_port)>(port));

    ::bind(handle, reinterpret_cast<struct sockaddr const*>(&addr), sizeof(addr));
}

void UdpSocket::allowBroadcasts() {
    int enable = 1;
    setsockopt(handle, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
}

void UdpSocket::send(uint8_t const* data, int size, IpAddress const& address, int port) const {
    struct sockaddr_in addr;

    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(address.rawAddress());
    addr.sin_port = htons(static_cast<decltype(addr.sin_port)>(port));

    sendto(handle, data, size, 0, reinterpret_cast<struct sockaddr const*>(&addr), sizeof(addr));
}

void UdpSocket::send(Data const& data, IpAddress const& address, int port) const {
    send(data.data(), data.size(), address, port);
}

UdpSocket::Data UdpSocket::receive(size_t maxPacketSize) const {
    Data data(maxPacketSize);

    struct sockaddr_in remoteAddr;
    socklen_t remoteAddrSize = sizeof(remoteAddr);

    int receivedBytes =
        recvfrom(handle, data.data(), data.size(), 0, reinterpret_cast<struct sockaddr*>(&remoteAddr), &remoteAddrSize);

    data.resize(receivedBytes);
    return data;
}
