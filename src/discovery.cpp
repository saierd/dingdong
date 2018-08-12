#include "discovery.h"

#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>

#include "network/interface.h"
#include "network/udp.h"

int const discoveryPort = 40001;
std::chrono::seconds const discoverySendInterval(1);
int const discoveryNameSize = 50;

struct DiscoveryMessage {
    DiscoveryMessage(Instance const& instance, NetworkInterface const& interface) : id(instance.id()) {
        std::strncpy(name, instance.name().c_str(), discoveryNameSize);
        ipAddress = interface.address().rawAddress();
    }

    Instance toInstance() const {
        return {
            id,
            name,
            IpAddress(ipAddress)
        };
    }

    MachineId id;
    char name[discoveryNameSize];
    std::uint32_t ipAddress;
};

// Periodically broadcasts discovery messages on all available interfaces.
void sendDiscoveryMessages(Instance const& self, std::chrono::seconds interval) {
    auto interfaces = getNetworkInterfaces();

    std::vector<UdpSocket> sockets(interfaces.size());
    std::vector<DiscoveryMessage> messages;
    messages.reserve(interfaces.size());
    for (std::size_t i = 0; i < interfaces.size(); i++) {
        sockets[i].allowBroadcasts();
        messages.emplace_back(self, interfaces[i]);
    }

    auto nextInterval = std::chrono::steady_clock::now() + interval;
    while (true) {
        for (std::size_t i = 0; i < interfaces.size(); i++) {
            sockets[i].sendStruct(messages[i], interfaces[i].broadcastAddress(), discoveryPort);
        }

        std::this_thread::sleep_until(nextInterval);
        nextInterval += interval;
    }
}

using DiscoveryCallback = std::function<void(Instance)>;
void listenForDiscoveries(DiscoveryCallback const& callback) {
    UdpSocket socket;
    socket.bind(discoveryPort);

    while (true) {
        auto data = socket.receive();
        if (data.size() == sizeof(DiscoveryMessage)) {
            auto discoveryMessage = reinterpret_cast<DiscoveryMessage const*>(data.data());
            callback(discoveryMessage->toInstance());
        }
    }
}

class InstanceDiscovery::Impl {
public:
    Impl(Instance const& self) : self(self) {}

    Instance self;

    std::mutex mutex;
    std::vector<Instance> instances;

    bool forceInstancesChangedSignal = false;
    std::vector<InstancesChangedCallback> callbacks;

    std::thread discoverySenderThread;
    std::thread discoveryReceiverThread;

    void addInstance(Instance const& newInstance) {
        std::lock_guard<std::mutex> lock(mutex);

        //if (newInstance.id() == self.id()) return;

        bool changed = false;
        bool found = false;
        for (auto& instance : instances) {
            if (instance.id() == newInstance.id()) {
                if (instance != newInstance) {
                    instance = newInstance;
                    changed = true;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            instances.push_back(newInstance);
            changed = true;
        }

        if (changed || forceInstancesChangedSignal) {
            for (auto const& callback : callbacks) {
                callback(instances);
            }
            forceInstancesChangedSignal = false;
        }
    }
};

InstanceDiscovery::InstanceDiscovery(Instance const& self) {
    impl = std::make_unique<Impl>(self);

    impl->discoverySenderThread = std::thread([this]() {
        sendDiscoveryMessages(impl->self, discoverySendInterval);
    });
    impl->discoveryReceiverThread = std::thread([this]() {
        listenForDiscoveries([this](Instance const& instance) {
            impl->addInstance(instance);
        });
    });
}

InstanceDiscovery::~InstanceDiscovery() {}

void InstanceDiscovery::onInstancesChanged(InstancesChangedCallback const& callback) {
    std::lock_guard<std::mutex> lock(impl->mutex);
    impl->forceInstancesChangedSignal = true;
    impl->callbacks.push_back(callback);
}

std::vector<Instance> InstanceDiscovery::instances() const {
    std::lock_guard<std::mutex> lock(impl->mutex);
    return impl->instances;
}
