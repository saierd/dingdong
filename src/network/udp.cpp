#include "udp.h"

#include <cstring>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <util/logging.h>

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

void UdpSocket::bind(int port, bool allowAddressReuse) {
    if (allowAddressReuse) {
        int value = 1;
        if (setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) == -1) {
            log()->error("Setting SO_REUSEADDR failed with error code {}", errno);
        }
    }

    struct sockaddr_in addr;

    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<decltype(addr.sin_port)>(port));

    if (::bind(handle, reinterpret_cast<struct sockaddr const*>(&addr), sizeof(addr)) == -1) {
        log()->error("Bind failed with error code {}", errno);
    }
}

void UdpSocket::allowBroadcasts() {
    int enable = 1;
    if (setsockopt(handle, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) == -1) {
        log()->error("Setting SO_BROADCAST failed with error code {}", errno);
    }
}

void UdpSocket::setReceiveTimeout(std::chrono::milliseconds timeout) {
    receiveTimeout = timeout;
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
    if (receiveTimeout.count() > 0) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(handle, &fds);

        struct timeval timeout;
        timeout.tv_sec = receiveTimeout.count() / 1000;
        timeout.tv_usec = (receiveTimeout.count() % 1000) * 1000;

        select(handle + 1, &fds, NULL, NULL, &timeout);
        if (!FD_ISSET(handle, &fds)) {
            // Socket has no data yet.
            return {};
        }
    }

    Data data(maxPacketSize);

    struct sockaddr_in remoteAddr;
    socklen_t remoteAddrSize = sizeof(remoteAddr);

    int receivedBytes =
        recvfrom(handle, data.data(), data.size(), 0, reinterpret_cast<struct sockaddr*>(&remoteAddr), &remoteAddrSize);

    data.resize(receivedBytes);
    return data;
}
