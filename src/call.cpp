#include "call.h"

#include <map>
#include <mutex>

#include "stream/audio.h"
#include "util/logging.h"

class PortManager {
public:
    class Handle {
    private:
        friend class PortManager;
        Handle(PortManager* _manager, int _port) : manager(_manager), port(_port) {}

    public:
        Handle() {
            invalidate();
        }

        ~Handle() {
            returnPort();
        }

        Handle(Handle const& other) = delete;
        Handle& operator=(Handle const& other) = delete;

        Handle(Handle&& other) {
            returnPort();
            manager = other.manager;
            port = other.port;
            other.invalidate();
        }

        Handle& operator=(Handle&& other) {
            returnPort();
            manager = other.manager;
            port = other.port;
            other.invalidate();
            return *this;
        }

    public:
        bool isValid() const {
            return manager != nullptr && port != 0;
        }

        int get() const {
            return port;
        }

    private:
        void returnPort() {
            if (manager != nullptr) {
                manager->returnPort(port);
            }
            invalidate();
        }

        void invalidate() {
            manager = nullptr;
            port = 0;
        }

        PortManager* manager;
        int port;
    };

public:
    PortManager(int minPort, int maxPort) {
        for (int port = minPort; port <= maxPort; port++) {
            availablePorts[port] = true;
        }
    }

    Handle getPort() {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& it : availablePorts) {
            if (it.second) {
                log()->trace("Handing out port {}", it.first);
                it.second = false;
                return Handle(this, it.first);
            }
        }

        return Handle();
    }

private:
    void returnPort(int port) {
        std::lock_guard<std::mutex> lock(mutex);
        log()->trace("Returning port {}", port);
        availablePorts[port] = true;
    }

    std::map<int, bool> availablePorts;
    std::mutex mutex;
};

int const callMinimumPort = 41000;
int const callMaximumPort = 41999;
static PortManager globalPortManager(callMinimumPort, callMaximumPort);

std::string const callLogCategory = "call";

class Call::Impl {
public:
    Impl(Settings const& settings, Instance const& _target) : target(_target) {
        audioSourceDevice = settings.audioSourceDevice();
        logger = categoryLogger(callLogCategory);
    }

    std::string audioSourceDevice;
    Instance target;
    UUID id;

    bool invalid = false;

    std::unique_ptr<AudioSender> sender;

    PortManager::Handle receiverPort;
    std::unique_ptr<AudioReceiver> receiver;

    Logger logger;
};

Call::Call(Settings const& settings, Instance const& target) {
    impl = std::make_unique<Impl>(settings, target);

    impl->receiverPort = globalPortManager.getPort();
    if (impl->receiverPort.isValid()) {
        impl->logger->info("Starting audio receiver on port {}", impl->receiverPort.get());
        impl->receiver = std::make_unique<AudioReceiver>(impl->receiverPort.get());
        impl->receiver->start();
    }
}

Call::Call(Settings const& settings, UUID const& id, Instance const& target) : Call(settings, target) {
    impl->id = id;
}

Call::~Call() {}  // Stops automatically in the destructor of the streams.

Call::Call(Call&& other) {
    impl = std::move(other.impl);
}

Call& Call::operator=(Call&& other) {
    impl = std::move(other.impl);
    return *this;
}

UUID Call::id() const {
    return impl->id;
}

Instance Call::target() const {
    return impl->target;
}

int Call::receiverPort() const {
    return impl->receiverPort.get();
}

void Call::connect(int senderPort) {
    impl->logger->info("Connecting audio sender to {}:{}", impl->target.ipAddress().toString(), senderPort);
    impl->sender = std::make_unique<AudioSender>(impl->target.ipAddress(), senderPort, impl->audioSourceDevice);
}

void Call::start() {
    if (impl->sender) {
        impl->logger->info("Starting sender for call {}", impl->id.toString());
        impl->sender->start();
    }
}

void Call::stop() {
    if (impl->sender) {
        impl->logger->info("Stopping sender for call {}", impl->id.toString());
        impl->sender->stop();
    }
}

bool Call::isRunning() const {
    return impl->sender && impl->sender->isRunning();
}

bool Call::isInvalid() const {
    return impl->invalid;
}

void Call::invalidate() {
    impl->invalid = true;
    stop();
    if (impl->receiver) {
        impl->receiver->stop();
    }
}
