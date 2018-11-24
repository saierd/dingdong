#include "discovery.h"

#include <chrono>
#include <cstring>
#include <map>
#include <mutex>
#include <thread>

#include "network/interface.h"
#include "network/udp.h"
#include "util/logging.h"

int const discoveryPort = 40001;
#ifdef NDEBUG
std::chrono::seconds const discoverySendInterval(10);
#else
std::chrono::seconds const discoverySendInterval(2);
#endif
int const discoveryNameSize = 50;

std::chrono::seconds const discoveryCleanupInterval(30);
std::chrono::seconds const discoveryTimeout(30);

std::string const discoveryLogCategory = "discovery";

struct DiscoveryMessage {
    DiscoveryMessage(Instance const& instance, NetworkInterface const& interface) : id(instance.id()) {
        std::strncpy(name, instance.name().c_str(), discoveryNameSize);
        ipAddress = interface.address().rawAddress();
    }

    Instance toInstance() const {
        return { id, name, IpAddress(ipAddress) };
    }

    MachineId id;
    char name[discoveryNameSize];
    std::uint32_t ipAddress;
};

// Periodically broadcasts discovery messages on all available interfaces.
void sendDiscoveryMessages(Instance const& self, std::chrono::seconds interval) {
    auto logger = categoryLogger(discoveryLogCategory);
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
            logger->debug("Broadcast discovery message on interface {} (IP {})", interfaces[i].name(),
                          interfaces[i].address().toString());
            sockets[i].sendStruct(messages[i], interfaces[i].broadcastAddress(), discoveryPort);
        }

        std::this_thread::sleep_until(nextInterval);
        nextInterval += interval;
    }
}

using DiscoveryCallback = std::function<void(Instance)>;
void listenForDiscoveries(DiscoveryCallback const& callback) {
    auto logger = categoryLogger(discoveryLogCategory);

    UdpSocket socket;
    socket.bind(discoveryPort);

    while (true) {
        auto data = socket.receive();
        if (data.size() == sizeof(DiscoveryMessage)) {
            auto discoveryMessage = reinterpret_cast<DiscoveryMessage const*>(data.data());
            auto instance = discoveryMessage->toInstance();
            logger->debug("Received discovery message from {} (IP {})", instance.id().toString(),
                          instance.ipAddress().toString());
            callback(instance);
        }
    }
}

class InstanceDiscovery::Impl {
public:
    Impl(Instance const& self) : self(self) {}

    Instance self;

    std::mutex mutex;
    std::vector<Instance> instances;
    std::map<std::string, std::chrono::steady_clock::time_point> instanceLastSeen;

    bool forceInstancesChangedSignal = false;
    std::vector<InstancesChangedCallback> callbacks;

    std::thread discoverySenderThread;
    std::thread discoveryReceiverThread;
    std::thread cleanupThread;

    void addInstance(Instance const& newInstance) {
        std::lock_guard<std::mutex> lock(mutex);

#ifdef NDEBUG
        // In debug mode, we discover ourselves for debugging purposes.
        if (newInstance.id() == self.id()) return;
#endif

        instanceLastSeen[newInstance.id().toString()] = std::chrono::steady_clock::now();

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
            instancesChanged();
            forceInstancesChangedSignal = false;
        }
    }

    // Remove timed out instances from the instance list.
    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex);

        auto now = std::chrono::steady_clock::now();
        size_t previousInstanceCount = instances.size();
        instances.erase(std::remove_if(instances.begin(), instances.end(),
                                       [&now, this](Instance const& instance) {
                                           if ((now - instanceLastSeen[instance.id().toString()]) > discoveryTimeout) {
                                               auto logger = categoryLogger(discoveryLogCategory);
                                               logger->debug("Instance {} ({}) timed out", instance.id().toString(),
                                                             instance.name());
                                               return true;
                                           }
                                           return false;
                                       }),
                        instances.end());

        for (auto it = instanceLastSeen.begin(); it != instanceLastSeen.end();) {
            bool instanceExists = false;
            for (auto const& instance : instances) {
                if (instance.id().toString() == it->first) {
                    instanceExists = true;
                    break;
                }
            }

            if (!instanceExists) {
                it = instanceLastSeen.erase(it);
            } else {
                it++;
            }
        }

        if (instances.size() < previousInstanceCount) {
            instancesChanged();
        }
    }

    void instancesChanged() {
        for (auto const& callback : callbacks) {
            callback(instances);
        }
    }
};

InstanceDiscovery::InstanceDiscovery(Instance const& self) {
    impl = std::make_unique<Impl>(self);

    impl->discoverySenderThread = std::thread([this]() { sendDiscoveryMessages(impl->self, discoverySendInterval); });
    impl->discoveryReceiverThread = std::thread(
        [this]() { listenForDiscoveries([this](Instance const& instance) { impl->addInstance(instance); }); });
    impl->cleanupThread = std::thread([this]() {
        auto nextInterval = std::chrono::steady_clock::now() + discoveryCleanupInterval;
        while (true) {
            impl->cleanup();
            std::this_thread::sleep_until(nextInterval);
            nextInterval += discoveryCleanupInterval;
        }
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
