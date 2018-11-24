#pragma once

#include <cstdint>
#include <string>

struct sockaddr;

class IpAddress {
public:
    IpAddress() : address(0) {}
    explicit IpAddress(std::uint32_t _address) : address(_address) {}
    explicit IpAddress(std::string const& addressString);
    explicit IpAddress(struct sockaddr const* addr);

    bool isZero() const {
        return address == 0;
    }

    std::uint32_t rawAddress() const {
        return address;
    }

    std::string toString() const;

    bool operator==(IpAddress const& other) const {
        return address == other.address;
    }

    bool operator!=(IpAddress const& other) const {
        return !(*this == other);
    }

private:
    // Address in host byte order.
    std::uint32_t address;
};
