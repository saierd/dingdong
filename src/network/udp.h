#pragma once

#include <vector>

#include "ip_address.h"

// The default ethernet packet size of 1500 bytes minus IP and UDP headers.
int const defaultMaxPacketSize = 1472;

class UdpSocket {
public:
    using Data = std::vector<uint8_t>;

public:
    UdpSocket();
    ~UdpSocket();

    UdpSocket(UdpSocket const&) = delete;
    UdpSocket& operator=(UdpSocket const&) = delete;
    UdpSocket(UdpSocket&& other) noexcept;
    UdpSocket& operator=(UdpSocket&& other) noexcept;

    void bind(int port = 0, bool allowAddressReuse = false);
    void allowBroadcasts();

    void send(uint8_t const* data, int size, IpAddress const& address, int port) const;
    void send(Data const& data, IpAddress const& address, int port) const;

    template<typename T>
    void sendStruct(T const& data, IpAddress const& address, int port) {
        send(reinterpret_cast<uint8_t const*>(&data), sizeof(T), address, port);
    }

    Data receive(size_t maxPacketSize = defaultMaxPacketSize) const;

private:
    int handle = 0;
};
