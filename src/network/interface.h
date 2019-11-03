#pragma once

#include <string>
#include <vector>

#include "ip_address.h"

class NetworkInterface {
public:
    NetworkInterface(std::string name, IpAddress const& address, IpAddress const& netmask,
                     IpAddress const& broadcastAddress)
        : _name(std::move(name)), _address(address), _netmask(netmask), _broadcastAddress(broadcastAddress) {}

public:
    std::string name() const {
        return _name;
    }

    IpAddress address() const {
        return _address;
    }

    IpAddress broadcastAddress() const {
        return _broadcastAddress;
    }

    bool isLoopback() const {
        return _address == IpAddress("127.0.0.1");
    }

    bool operator==(NetworkInterface const& other) const {
        return _name == other._name && _address == other._address && _netmask == other._netmask &&
               _broadcastAddress == other._broadcastAddress;
    }

    bool operator!=(NetworkInterface const& other) const {
        return !(*this == other);
    }

private:
    std::string _name;
    IpAddress _address;
    IpAddress _netmask;
    IpAddress _broadcastAddress;
};

std::vector<NetworkInterface> getNetworkInterfaces(bool includeLoopback = false);
