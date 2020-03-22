#include "call_protocol.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "httplib.h"

#include "audio_manager.h"
#include "call.h"
#include "util/json.h"
#include "util/logging.h"

std::string const protocolLoggingCategory = "http";
int const protocolPort = 40002;

std::chrono::seconds const callCleanupInterval(2);
std::chrono::seconds const callPingInterval(1);
std::chrono::seconds const callInactivityTimeout(5);
std::chrono::seconds const callAcceptTimeout(45);
std::chrono::minutes const callTimeout(3);

char const* jsonContentType = "application/json";

httplib::Client clientForTarget(Instance const& target) {
    return httplib::Client(target.ipAddress().toString().c_str(), protocolPort);
}

class CallProtocol::Impl {
public:
    Impl(CallProtocol* _protocol, Settings _self, InstanceDiscovery const& _instances,
         std::shared_ptr<AudioManager> _audioManager)
        : protocol(_protocol), self(std::move(_self)), instances(_instances), audioManager(std::move(_audioManager)) {
        logger = categoryLogger(protocolLoggingCategory);

        httpServer.Post("/call/request", [this](httplib::Request const& request, httplib::Response& response) {
            auto data = Json::parse(request.body);
            UUID id(data["id"].get<std::string>());
            MachineId machine(data["machine"].get<std::string>());
            int senderPort = data["port"];

            bool foundInstance = false;
            std::optional<int> receiverPort;
            {
                std::lock_guard<std::mutex> lock(mutex);
                logger->info("Call request from {}, call id {}", machine.toString(), id.toString());

                playRingtone();

                for (auto const& instance : instances.instances()) {
                    if (instance.id() == machine) {
                        auto& newCall = createIncomingCall(id, instance);
                        newCall.connect(senderPort);

                        Json result;
                        result["id"] = newCall.id().toString();
                        result["port"] = newCall.receiverPort();
                        response.set_content(result.dump(), jsonContentType);

                        receiverPort = newCall.receiverPort();
                        foundInstance = true;

                        break;
                    }
                }
            }

            if (foundInstance) {
                protocol->onCallsChanged();
            } else {
                logger->error("Could not find instance {}", machine.toString());
                response.status = 500;
            }

            if (self.autoAccept()) {
                protocol->acceptCall(id, receiverPort);
            }
        });
        httpServer.Post("/call/accept", [this](httplib::Request const& request, httplib::Response& response) {
            auto data = Json::parse(request.body);
            UUID id(data["id"].get<std::string>());

            {
                std::lock_guard<std::mutex> lock(mutex);
                logger->info("Received accept request for {}", id.toString());

                auto call = outgoingCallById(id);
                if (call == nullptr) {
                    logger->error("Could not find call {}", id.toString());
                    response.status = 500;
                    return;
                }

                callAcceptTime[call->id().toString()] = std::chrono::steady_clock::now();

                // Optionally connect the accepted call if the accept API call was given a port. This is to allow auto
                // accepting calls while the request on the calling side was not completed yet.
                if (data.count("port")) {
                    int port = data["port"];
                    call->connect(port);
                }

                call->start();
            }

            protocol->onCallsChanged();

            Json result;
            result["status"] = "ok";
            response.set_content(result.dump(), jsonContentType);
        });
        httpServer.Post("/call/cancel", [this](httplib::Request const& request, httplib::Response& response) {
            auto data = Json::parse(request.body);
            UUID id(data["id"].get<std::string>());

            logger->info("Received cancel request for {}", id.toString());
            if (!invalidateCall(id)) {
                logger->error("Could not find call {}", id.toString());
                response.status = 500;
                return;
            }

            protocol->onCallsChanged();

            Json result;
            result["status"] = "ok";
            response.set_content(result.dump(), jsonContentType);
        });
        httpServer.Post("/call/ping", [this](httplib::Request const& request, httplib::Response& response) {
            auto data = Json::parse(request.body);
            UUID id(data["id"].get<std::string>());

            {
                std::lock_guard<std::mutex> lock(mutex);
                logger->trace("Received ping for {}", id.toString());
                callLastActivity[id.toString()] = std::chrono::steady_clock::now();
            }

            Json result;
            result["status"] = "ok";
            response.set_content(result.dump(), jsonContentType);
        });
        httpServer.Get("/ring", [this](httplib::Request const& /*unused*/, httplib::Response& response) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                playRingtone();
            }

            Json result;
            result["status"] = "ok";
            response.set_content(result.dump(), jsonContentType);
        });
        httpServer.Post("/action", [this](httplib::Request const& request, httplib::Response& response) {
            auto data = Json::parse(request.body);
            auto action = data["id"].get<std::string>();

            logger->info("Received request for action {}", action);
            protocol->onActionRequested(action);

            Json result;
            result["status"] = "ok";
            response.set_content(result.dump(), jsonContentType);
        });
        httpServerThread = std::thread([&]() {
            logger->debug("Start HTTP server on port {}", protocolPort);
            httpServer.listen("0.0.0.0", protocolPort);
        });

        pingThread = std::thread([this, stop = &stopThreads]() {
            auto nextInterval = std::chrono::steady_clock::now() + callPingInterval;
            while (!stop->load()) {
                pingCalls();
                std::this_thread::sleep_until(nextInterval);
                nextInterval += callPingInterval;
            }
        });

        cleanupThread = std::thread([this, stop = &stopThreads]() {
            auto nextInterval = std::chrono::steady_clock::now() + callCleanupInterval;
            while (!stop->load()) {
                cleanup();
                std::this_thread::sleep_until(nextInterval);
                nextInterval += callCleanupInterval;
            }
        });
    }

    ~Impl() {
        stopThreads = true;

        if (cleanupThread.joinable()) {
            cleanupThread.join();
        }
        if (pingThread.joinable()) {
            pingThread.join();
        }

        httpServer.stop();
        if (httpServerThread.joinable()) {
            httpServerThread.join();
        }
    }

    Call& createIncomingCall(UUID const& id, Instance const& instance) {
        incomingCalls.emplace_back(self, id, instance, audioManager);

        auto& newCall = incomingCalls.back();
        callLastActivity[newCall.id().toString()] = std::chrono::steady_clock::now();
        callCreationTime[newCall.id().toString()] = std::chrono::steady_clock::now();

        return newCall;
    }

    Call& createOutgoingCall(Instance const& target) {
        outgoingCalls.emplace_back(self, target, audioManager);

        auto& newCall = outgoingCalls.back();
        logger->debug("Created call with id {}", newCall.id().toString());
        callLastActivity[newCall.id().toString()] = std::chrono::steady_clock::now();
        callCreationTime[newCall.id().toString()] = std::chrono::steady_clock::now();

        return newCall;
    }

    Call* incomingCallById(UUID id) {
        for (auto& call : incomingCalls) {
            if (call.id() == id) {
                return &call;
            }
        }
        return nullptr;
    }

    Call* outgoingCallById(UUID id) {
        for (auto& call : outgoingCalls) {
            if (call.id() == id) {
                return &call;
            }
        }
        return nullptr;
    }

    void cancelCall(UUID id, bool needLock = true) {
        Instance callTarget(getMachineId(), "");
        if (invalidateCall(id, &callTarget, needLock)) {
            Json data;
            data["id"] = id.toString();

            auto request = clientForTarget(callTarget);
            auto response = request.Post("/call/cancel", data.dump(), jsonContentType);
        }

        protocol->onCallsChanged();
    }

    // Canceling calls can happen from both the caller and the callee side. On
    // the event, we therefore don't know whether the call is an incoming or
    // outgoing one, so we search both.
    // Note that for the degenerate case of calling ourselves, this function
    // invalidates both the incoming and outgoing instances of the call.
    bool invalidateCall(UUID id, Instance* target = nullptr, bool needLock = true) {
        std::unique_lock<std::mutex> lock(mutex, std::defer_lock);
        if (needLock) {
            lock.lock();
        }

        bool found = false;

        auto call = incomingCallById(id);
        if (call != nullptr) {
            call->invalidate();
            if (target != nullptr) *target = call->target();
            found = true;
        }

        call = outgoingCallById(id);
        if (call != nullptr) {
            call->invalidate();
            if (target != nullptr) *target = call->target();
            found = true;
        }

        return found;
    }

    // Ping the partner instance for all active calls.
    void pingCalls() {
        // Make a copy of the calls to ping to avoid deadlocks when another
        // instance is pinging at the same time (the HTTP ping callback needs
        // to lock as well).
        std::vector<std::pair<Instance, UUID>> callsToPing;
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto const& call : incomingCalls) {
                if (call.isInvalid()) continue;
                callsToPing.emplace_back(call.target(), call.id());
            }
            for (auto const& call : outgoingCalls) {
                if (call.isInvalid()) continue;
                callsToPing.emplace_back(call.target(), call.id());
            }
        }

        for (auto const& call : callsToPing) {
            Json data;
            data["id"] = call.second.toString();

            auto request = clientForTarget(call.first);
            request.Post("/call/ping", data.dump(), jsonContentType);
        }
    }

    // Remove canceled and timed out calls.
    void cleanup() {
        std::unique_lock<std::mutex> lock(mutex);

        auto now = std::chrono::steady_clock::now();
        size_t previousCallCount = incomingCalls.size() + outgoingCalls.size();

        // Cancel outgoing calls that have not been accepted for some time.
        for (auto const& call : outgoingCalls) {
            if (call.isInvalid() || call.isRunning()) continue;

            if ((now - callCreationTime[call.id().toString()]) > callAcceptTimeout) {
                logger->info("Cancel call {} due to accept timeout", call.id().toString());
                cancelCall(call.id(), false);
            }
        }

        // Cancel calls that went too long.
        auto cancelTimedOutCalls = [&now, this](std::vector<Call>& calls) {
            for (auto const& call : calls) {
                if (call.isInvalid() || !call.isRunning()) continue;

                auto acceptTime = callAcceptTime.find(call.id().toString());
                if (acceptTime == callAcceptTime.end()) continue;

                if ((now - acceptTime->second) > callTimeout) {
                    logger->info("Cancel call {} due to timeout", call.id().toString());
                    cancelCall(call.id(), false);
                }
            }
        };

        cancelTimedOutCalls(incomingCalls);
        cancelTimedOutCalls(outgoingCalls);

        auto cleanupCallList = [&now, this](std::vector<Call>& calls) {
            calls.erase(std::remove_if(calls.begin(), calls.end(),
                                       [&now, this](Call const& call) {
                                           if (call.isInvalid()) return true;

                                           if ((now - callLastActivity[call.id().toString()]) > callInactivityTimeout) {
                                               logger->debug("Call {} timed out due to inactivity",
                                                             call.id().toString());
                                               return true;
                                           }

                                           return false;
                                       }),
                        calls.end());
        };

        cleanupCallList(incomingCalls);
        cleanupCallList(outgoingCalls);

        auto cleanCallMap = [this](std::map<std::string, std::chrono::steady_clock::time_point>& map) {
            for (auto it = map.begin(); it != map.end();) {
                UUID id(it->first);
                if (outgoingCallById(id) == nullptr && incomingCallById(id) == nullptr) {
                    it = map.erase(it);
                } else {
                    it++;
                }
            }
        };

        cleanCallMap(callCreationTime);
        cleanCallMap(callLastActivity);

        lock.unlock();
        if (incomingCalls.size() + outgoingCalls.size() < previousCallCount) {
            protocol->onCallsChanged();
        }
    }

    void playRingtone() {
        audioManager->playRingtone(self.ringtone());
    }

public:
    Logger logger;

    CallProtocol* protocol;

    Settings self;
    InstanceDiscovery const& instances;

    std::shared_ptr<AudioManager> audioManager;

    std::mutex mutex;

    // We store incoming and outgoing calls separately, so that we can call
    // ourselves for debugging purposes (in which case we will have both an
    // incoming and an outgoing call with the same id). This wouldn't be
    // necessary when we only call other instances.
    std::vector<Call> incomingCalls;
    std::vector<Call> outgoingCalls;

    // For each call in one of the lists above (by their id as a string), the
    // time at which it was created and the time when we last received a ping
    // for that call.
    std::map<std::string, std::chrono::steady_clock::time_point> callCreationTime;
    std::map<std::string, std::chrono::steady_clock::time_point> callLastActivity;
    std::map<std::string, std::chrono::steady_clock::time_point> callAcceptTime;

    httplib::Server httpServer;
    std::thread httpServerThread;

    std::atomic<bool> stopThreads = false;
    std::thread pingThread;
    std::thread cleanupThread;
};

CallProtocol::CallProtocol(Settings const& self, InstanceDiscovery const& instances,
                           std::shared_ptr<AudioManager> audioManager) {
    impl = std::make_unique<Impl>(this, self, instances, std::move(audioManager));
}

CallProtocol::~CallProtocol() = default;

void CallProtocol::requestCall(Instance const& target) {
    impl->logger->debug("Request call to {}", target.id().toString());

    bool alreadyCalled = false;
    bool callAlreadyRunning = false;
    {
        std::lock_guard<std::mutex> lock(impl->mutex);
        for (auto const& call : impl->outgoingCalls) {
            if (call.target().id() == target.id() && !call.isInvalid()) {
                alreadyCalled = true;
                callAlreadyRunning = call.isRunning();
                break;
            }
        }
        if (!alreadyCalled) {
            for (auto const& call : impl->incomingCalls) {
                if (call.target().id() == target.id() && !call.isInvalid()) {
                    alreadyCalled = true;
                    callAlreadyRunning = call.isRunning();
                    break;
                }
            }
        }
    }
    if (alreadyCalled) {
        // We already have a call to the target instance. Ring again. This has to happen without having the call mutex
        // locked to avoid a deadlock.
        if (!callAlreadyRunning) {
            auto request = clientForTarget(target);
            request.Get("/ring");
        }
        return;
    }

    Json data;
    UUID newCallId;
    {
        std::lock_guard<std::mutex> lock(impl->mutex);
        auto& newCall = impl->createOutgoingCall(target);

        newCallId = newCall.id();
        data["id"] = newCallId.toString();
        data["port"] = newCall.receiverPort();
        data["machine"] = impl->self.id().toString();
    }

    auto request = clientForTarget(target);
    auto response = request.Post("/call/request", data.dump(), jsonContentType);
    if (!response || response->status != 200) {
        impl->logger->error("Error while sending request");
        return;
    }

    impl->logger->trace("Response {}: {}", response->status, response->body);
    data = Json::parse(response->body);

    {
        std::lock_guard<std::mutex> lock(impl->mutex);
        auto call = impl->outgoingCallById(newCallId);
        if (call != nullptr) {
            call->connect(data["port"]);
        }
    }

    onCallsChanged();
}

void CallProtocol::acceptCall(UUID const& id, std::optional<int> receiverPort) {
    impl->logger->debug("Accept call {}", id.toString());

    bool success = true;
    std::unique_ptr<Instance> callTarget;
    {
        std::lock_guard<std::mutex> lock(impl->mutex);
        auto call = impl->incomingCallById(id);
        if (call != nullptr) {
            callTarget = std::make_unique<Instance>(call->target());
        }
    }

    if (callTarget) {
        Json data;
        data["id"] = id.toString();
        if (receiverPort) {
            data["port"] = *receiverPort;
        }

        auto request = clientForTarget(*callTarget);
        auto response = request.Post("/call/accept", data.dump(), jsonContentType);
        if (!response || response->status != 200) {
            impl->logger->error("Error while sending request");
            success = false;
        }

        if (success) {
            impl->callAcceptTime[callTarget->id().toString()] = std::chrono::steady_clock::now();
        }
    } else {
        success = false;
    }

    {
        std::lock_guard<std::mutex> lock(impl->mutex);
        auto call = impl->incomingCallById(id);
        if (success && call != nullptr) {
            call->start();
        } else if (call != nullptr) {
            call->invalidate();
        }
    }

    onCallsChanged();
}

void CallProtocol::cancelCall(UUID const& id) {
    impl->logger->debug("Cancel call {}", id.toString());
    impl->cancelCall(id);
}

void CallProtocol::muteCall(UUID const& id, bool mute) {
    std::unique_lock<std::mutex> lock(impl->mutex);

    auto call = impl->incomingCallById(id);
    if (call == nullptr) {
        call = impl->outgoingCallById(id);
    }

    if (call != nullptr) {
        if (mute && !call->isMuted()) {
            impl->logger->info("Muting call {}", id.toString());
            call->mute();
        } else if (call->isMuted()) {
            impl->logger->info("Unmuting call {}", id.toString());
            call->unmute();
        }

        onCallsChanged();
    }
}

void CallProtocol::requestRemoteAction(UUID const& callId, std::string const& actionId) {
    impl->logger->debug("Request remote action {} for call {}", actionId, callId.toString());

    auto call = impl->incomingCallById(callId);
    if (call == nullptr) {
        call = impl->outgoingCallById(callId);
    }

    if (call) {
        Json data;
        data["id"] = actionId;

        auto request = clientForTarget(call->target());
        auto response = request.Post("/action", data.dump(), jsonContentType);
        if (!response || response->status != 200) {
            impl->logger->error("Error while sending request");
        }
    }
}

std::vector<CallInfo> CallProtocol::currentActiveCalls() const {
    std::lock_guard<std::mutex> lock(impl->mutex);

    std::vector<CallInfo> result;
    auto addCallsFromList = [&result](std::vector<Call> const& calls, bool canBeAccepted) {
        for (auto const& call : calls) {
            if (call.isInvalid()) continue;

            result.push_back({ call.id(), call.target().id(), call.target().name(), call.isRunning(), call.isMuted(),
                               canBeAccepted && !call.isRunning(), call.target().remoteActions() });
        }
    };

    addCallsFromList(impl->incomingCalls, true);
    addCallsFromList(impl->outgoingCalls, false);

    return result;
}
