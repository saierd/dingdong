#pragma once

#include <string>
#include <vector>

#include "ip_address.h"

class NetworkInterface {
public:
    NetworkInterface(std::string const& name, IpAddress const& address,
                     IpAddress const& netmask, IpAddress const& broadcastAddress)
        : _name(name), _address(address), _netmask(netmask), _broadcastAddress(broadcastAddress) {}

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

private:
    std::string _name;
    IpAddress _address;
    IpAddress _netmask;
    IpAddress _broadcastAddress;
};

std::vector<NetworkInterface> getNetworkInterfaces(bool includeLoopback = false);
