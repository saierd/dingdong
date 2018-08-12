#pragma once

#include <vector>

#include "ip_address.h"

class UdpSocket {
public:
    using Data = std::vector<unsigned char>;

public:
    UdpSocket();
    ~UdpSocket();

    UdpSocket(UdpSocket const&) = delete;
    UdpSocket& operator=(UdpSocket const&) = delete;
    UdpSocket(UdpSocket&& other);
    UdpSocket& operator=(UdpSocket&& other);

    void bind(int port = 0);
    void allowBroadcasts();

    void send(unsigned char const* data, int size, IpAddress const& address, int port) const;
    void send(Data const& data, IpAddress const& address, int port) const;

    template<typename T>
    void sendStruct(T const& data, IpAddress const& address, int port) {
        send(reinterpret_cast<unsigned char const*>(&data), sizeof(T), address, port);
    }

    Data receive(size_t maxPacketSize = 1472) const;

private:
    int handle = 0;
};
