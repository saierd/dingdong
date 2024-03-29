#include "ip_address.h"

#include <array>

#include <arpa/inet.h>

IpAddress::IpAddress(std::string const& addressString) {
    in_addr addr;
    inet_pton(AF_INET, addressString.c_str(), &addr);
    address = ntohl(addr.s_addr);
}

IpAddress::IpAddress(struct sockaddr const* addr) {
    if (addr->sa_family != AF_INET) {
        address = 0;
        return;
    }

    auto const* addr_in = reinterpret_cast<sockaddr_in const*>(addr);
    address = ntohl(addr_in->sin_addr.s_addr);
}

std::string IpAddress::toString() const {
    std::array<char, INET_ADDRSTRLEN> buffer;

    in_addr addr;
    addr.s_addr = htonl(address);
    inet_ntop(AF_INET, &addr, buffer.data(), INET_ADDRSTRLEN);

    return std::string(buffer.data());
}
