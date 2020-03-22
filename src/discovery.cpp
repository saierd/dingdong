#include "discovery.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

#include "network/interface.h"
#include "network/udp.h"
#include "util/json.h"
#include "util/logging.h"

int const discoveryPort = 40001;

std::chrono::seconds const discoverySendInterval(1);
std::chrono::seconds const discoveryCleanupInterval(10);
std::chrono::seconds const discoveryTimeout(60);

// Discovery broadcasts are a UDP packet consisting of this ASCII string followed by data in the form of ASCII encoded
// JSON. This allows reading the messages in Wireshark as well as extending them easily.
std::string const discoveryBroadcastIdentifier = "DINGDONG_DISCOVERY_V1";

std::string const discoveryFieldId = "id";
std::string const discoveryFieldName = "name";
std::string const discoveryFieldIp = "ip";
std::string const discoveryFieldOrder = "order";
std::string const discoveryFieldActions = "actions";
std::string const discoveryFieldCanReceiveVideo = "can_receive_video";

std::string const discoveryLogCategory = "discovery";

class DiscoveryMessage {
public:
    explicit DiscoveryMessage(std::string_view const& serializedData) {
        data = Json::parse(serializedData);
    }

    DiscoveryMessage(Instance const& instance, NetworkInterface const& interface) {
        data[discoveryFieldId] = instance.id().toString();
        data[discoveryFieldName] = instance.name();
        data[discoveryFieldIp] = interface.address().toString();
        data[discoveryFieldOrder] = instance.order();
        data[discoveryFieldCanReceiveVideo] = instance.canReceiveVideo();

        if (!instance.remoteActions().empty()) {
            for (auto const& action : instance.remoteActions()) {
                data[discoveryFieldActions][action.id] = action.caption;
            }
        }
    }

    Instance toInstance() const {
        std::vector<Instance::RemoteAction> actions;
        if (data.find(discoveryFieldActions) != data.end()) {
            actions.reserve(data[discoveryFieldActions].size());
            for (auto const& [id, caption] : data[discoveryFieldActions].items()) {
                actions.emplace_back(Instance::RemoteAction{ id, caption });
            }
        }

        int order = 0;
        if (data.find(discoveryFieldOrder) != data.end()) {
            order = data[discoveryFieldOrder];
        }

        bool canReceiveVideo = false;
        if (data.find(discoveryFieldCanReceiveVideo) != data.end()) {
            canReceiveVideo = data[discoveryFieldCanReceiveVideo];
        }

        return { MachineId(data[discoveryFieldId]),
                 data[discoveryFieldName].get<std::string>(),
                 IpAddress(data[discoveryFieldIp].get<std::string>()),
                 order,
                 canReceiveVideo,
                 std::move(actions) };
    }

    // Serialize the discovery message to a UDP packet.
    std::vector<uint8_t> serialize() const {
        std::string serializedData = discoveryBroadcastIdentifier + data.dump();
        return { serializedData.begin(), serializedData.end() };
    }

private:
    Json data;
};

// Periodically broadcasts discovery messages on all available interfaces.
void sendDiscoveryMessages(Instance const& self, std::chrono::seconds interval, std::atomic<bool> const* stop) {
    auto logger = categoryLogger(discoveryLogCategory);

    std::vector<NetworkInterface> interfaces;
    std::vector<UdpSocket> sockets;
    std::vector<UdpSocket::Data> messages;

    auto updateInterfaces = [&]() {
        auto newInterfaces = getNetworkInterfaces();
        if (interfaces == newInterfaces) return;

        interfaces.clear();
        sockets.clear();
        messages.clear();

        interfaces = std::move(newInterfaces);
        sockets.resize(interfaces.size());
        messages.reserve(interfaces.size());
        for (std::size_t i = 0; i < interfaces.size(); i++) {
            sockets[i].allowBroadcasts();

            DiscoveryMessage message(self, interfaces[i]);
            messages.emplace_back(message.serialize());
        }
    };

    auto nextInterval = std::chrono::steady_clock::now() + interval;
    while (!stop->load()) {
        try {
            updateInterfaces();

            for (std::size_t i = 0; i < interfaces.size(); i++) {
                logger->debug("Broadcast discovery message on interface {} (IP {})", interfaces[i].name(),
                              interfaces[i].address().toString());
                sockets[i].send(messages[i], interfaces[i].broadcastAddress(), discoveryPort);
            }
        } catch (...) {
            logger->error("Caught error in discovery thread");
        }

        std::this_thread::sleep_until(nextInterval);
        nextInterval += interval;
    }
}

using DiscoveryCallback = std::function<void(Instance)>;

void listenForDiscoveries(DiscoveryCallback const& callback, std::atomic<bool> const* stop) {
    auto logger = categoryLogger(discoveryLogCategory);

    UdpSocket socket;
    socket.bind(discoveryPort, true);

    while (!stop->load()) {
        auto data = socket.receive();

        auto stringFromData = [&](size_t offset, size_t size) -> std::string_view {
            return std::string_view(reinterpret_cast<char const*>(data.data()) + offset, size);
        };

        size_t identifierSize = discoveryBroadcastIdentifier.size();

        if (data.size() >= discoveryBroadcastIdentifier.size() &&
            stringFromData(0, identifierSize) == discoveryBroadcastIdentifier) {
            auto json = stringFromData(identifierSize, data.size() - identifierSize);
            DiscoveryMessage message(json);
            auto instance = message.toInstance();
            logger->debug("Received discovery message from {} (IP {})", instance.id().toString(),
                          instance.ipAddress().toString());
            callback(instance);
        }
    }
}

class InstanceDiscovery::Impl {
public:
    Impl(Instance _self) : self(std::move(_self)) {}

    Instance self;

    std::mutex mutex;
    std::vector<Instance> instances;
    std::map<std::string, std::chrono::steady_clock::time_point> instanceLastSeen;

    bool forceInstancesChangedSignal = false;
    std::vector<InstancesChangedCallback> callbacks;

    std::atomic<bool> stopThreads = false;
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

        if (changed) {
            std::sort(instances.begin(), instances.end(),
                      [](Instance const& a, Instance const& b) { return a.order() < b.order(); });
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
            callback();
        }
    }
};

InstanceDiscovery::InstanceDiscovery(Instance const& self) {
    impl = std::make_unique<Impl>(self);

    impl->discoverySenderThread = std::thread(
        [this, stop = &impl->stopThreads]() { sendDiscoveryMessages(impl->self, discoverySendInterval, stop); });
    impl->discoveryReceiverThread = std::thread([this, stop = &impl->stopThreads]() {
        listenForDiscoveries([this](Instance const& instance) { impl->addInstance(instance); }, stop);
    });
    impl->cleanupThread = std::thread([this, stop = &impl->stopThreads]() {
        auto nextInterval = std::chrono::steady_clock::now() + discoveryCleanupInterval;
        while (!stop->load()) {
            impl->cleanup();
            std::this_thread::sleep_until(nextInterval);
            nextInterval += discoveryCleanupInterval;
        }
    });
}

InstanceDiscovery::~InstanceDiscovery() {
    impl->stopThreads = true;

    if (impl->discoverySenderThread.joinable()) {
        impl->discoverySenderThread.join();
    }
    if (impl->discoveryReceiverThread.joinable()) {
        impl->discoveryReceiverThread.join();
    }
    if (impl->cleanupThread.joinable()) {
        impl->cleanupThread.join();
    }
}

void InstanceDiscovery::onInstancesChanged(InstancesChangedCallback const& callback) {
    std::lock_guard<std::mutex> lock(impl->mutex);
    impl->forceInstancesChangedSignal = true;
    impl->callbacks.push_back(callback);
}

std::vector<Instance> InstanceDiscovery::instances() const {
    std::lock_guard<std::mutex> lock(impl->mutex);
    return impl->instances;
}
