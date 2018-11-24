#include "interface.h"

#include <ifaddrs.h>

std::vector<NetworkInterface> getNetworkInterfaces(bool includeLoopback) {
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1) {
        return {};
    }

    std::vector<NetworkInterface> interfaces;
    for (struct ifaddrs const* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        if (ifa->ifa_addr->sa_family != AF_INET) continue;

        interfaces.emplace_back(ifa->ifa_name, IpAddress(ifa->ifa_addr), IpAddress(ifa->ifa_netmask),
                                IpAddress(ifa->ifa_ifu.ifu_broadaddr));
        if (!includeLoopback && interfaces.back().isLoopback()) {
            interfaces.pop_back();
        }
    }

    freeifaddrs(ifaddr);

    return interfaces;
}
