#include "call.h"

#include <map>
#include <mutex>

#include "audio_manager.h"
#include "call_history.h"
#include "stream/audio.h"
#include "stream/video.h"
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

        Handle(Handle&& other) noexcept {
            *this = std::move(other);
        }

        Handle& operator=(Handle&& other) noexcept {
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
    Impl(Settings const& self, CallHistory* _history, Instance _target, std::shared_ptr<AudioManager> _audioManager)
        : target(std::move(_target)), history(_history), audioManager(std::move(_audioManager)) {
        videoSenderDevice = self.videoDevice();
        logger = categoryLogger(callLogCategory);
    }

    Instance target;
    UUID id;

    CallHistory* history;

    bool invalid = false;
    bool muted = false;
    bool accepted = false;
    bool canceled = false;

    std::string imageData;

    std::shared_ptr<AudioManager> audioManager;

    std::unique_ptr<AudioSender> sender;
    int senderPort = -1;

    PortManager::Handle receiverPort;
    std::unique_ptr<AudioReceiver> receiver;

    PortManager::Handle videoReceiverPort;
    bool remoteSendsVideo = false;
    std::shared_ptr<VideoReceiver> videoReceiver;

    std::unique_ptr<VideoSender> videoSender;
    std::string videoSenderDevice;
    int videoSenderPort = -1;

    Logger logger;
};

Call::Call(Settings const& self, CallHistory* history, Instance const& target,
           std::shared_ptr<AudioManager> audioManager) {
    impl = std::make_unique<Impl>(self, history, target, std::move(audioManager));

    impl->receiverPort = globalPortManager.getPort();
    if (impl->receiverPort.isValid()) {
        impl->logger->info("Starting audio receiver on port {}", impl->receiverPort.get());
        impl->receiver = std::make_unique<AudioReceiver>(impl->receiverPort.get(), self.callVolume());
        impl->receiver->start();
    }

    impl->videoReceiverPort = globalPortManager.getPort();
    if (impl->videoReceiverPort.isValid()) {
        impl->logger->info("Starting video receiver on port {}", impl->videoReceiverPort.get());
        impl->videoReceiver = std::make_shared<VideoReceiver>(impl->videoReceiverPort.get());
        impl->videoReceiver->start();
    }
}

Call::Call(Settings const& self, CallHistory* history, UUID const& id, Instance const& target,
           std::shared_ptr<AudioManager> audioManager)
    : Call(self, history, target, std::move(audioManager)) {
    impl->id = id;
}

Call::~Call() {
    if (impl) {
        stop();

        if (impl->history) {
            impl->history->addEntry(impl->accepted, impl->canceled, impl->imageData);
        }
    }
}

Call::Call(Call&& other) noexcept {
    impl = std::move(other.impl);
}

Call& Call::operator=(Call&& other) noexcept {
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
    if (impl->sender && senderPort == impl->senderPort) return;

    impl->logger->info("Connecting audio sender to {}:{}", impl->target.ipAddress().toString(), senderPort);
    impl->sender = std::make_unique<AudioSender>(impl->target.ipAddress(), senderPort);
    impl->senderPort = senderPort;
}

void Call::start() {
    impl->accepted = true;
    impl->audioManager->startCall();

    if (impl->sender) {
        impl->logger->info("Starting sender for call {}", impl->id.toString());
        impl->sender->start();
    }
    impl->muted = false;
}

void Call::stop() {
    impl->audioManager->stopCall();

    if (impl->sender) {
        impl->logger->info("Stopping sender for call {}", impl->id.toString());
        impl->sender->stop();
    }
}

void Call::mute() {
    stop();
    impl->muted = true;
}

void Call::unmute() {
    start();
    impl->muted = false;
}

bool Call::isRunning() const {
    return isMuted() || (impl->sender && impl->sender->isRunning());
}

bool Call::isMuted() const {
    return impl->muted;
}

void Call::setRemoteSendsVideo(bool remoteSendsVideo) {
    impl->remoteSendsVideo = remoteSendsVideo;
}

bool Call::remoteSendsVideo() const {
    return impl->remoteSendsVideo;
}

int Call::videoReceiverPort() const {
    return impl->videoReceiverPort.get();
}

std::shared_ptr<VideoReceiver> Call::videoReceiver() const {
    return impl->videoReceiver;
}

void Call::connectVideo(int senderPort) {
    if (impl->videoSender && senderPort == impl->videoSenderPort) return;

    impl->logger->info("Connecting video sender to {}:{}", impl->target.ipAddress().toString(), senderPort);
    impl->videoSender = std::make_unique<VideoSender>(impl->videoSenderDevice, impl->target.ipAddress(), senderPort);
    impl->videoSenderPort = senderPort;
}

void Call::startVideo() {
    if (impl->videoSender) {
        impl->videoSender->start();
    }
}

void Call::stopVideo() {
    if (impl->videoSender) {
        impl->videoSender->stop();
    }
}

bool Call::canSendVideo() const {
    return !impl->videoSenderDevice.empty();
}

bool Call::isSendingVideo() const {
    return impl->videoSender && impl->videoSender->isRunning();
}

void Call::setImageData(std::string data) {
    impl->imageData = std::move(data);
}

void Call::setCanceled() {
    impl->canceled = true;
}

bool Call::isInvalid() const {
    return impl->invalid;
}

void Call::invalidate() {
    impl->invalid = true;
    stop();
    stopVideo();
    if (impl->receiver) {
        impl->receiver->stop();
    }
    if (impl->videoReceiver) {
        impl->videoReceiver->stop();
    }
}
