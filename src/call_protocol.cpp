#include "call_protocol.h"

#include <algorithm>
#include <thread>
#include <mutex>
#include <vector>

#include "httplib.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

#include "call.h"
#include "util/logging.h"

std::string const protocolLoggingCategory = "http";
int const protocolPort = 40002;

char const* jsonContentType = "application/json";

httplib::Client clientForTarget(Instance const& target) {
    return httplib::Client(target.ipAddress().toString().c_str(), protocolPort);
}

class CallProtocol::Impl {
public:
    Impl(CallProtocol* _protocol, Instance const& _self, InstanceDiscovery const& _instances) : protocol(_protocol), self(_self), instances(_instances) {
        logger = categoryLogger(protocolLoggingCategory);

        httpServer.Post("/call/request", [this](httplib::Request const& request, httplib::Response& response) {
            auto data = json::parse(request.body);
            UUID id(data["id"].get<std::string>());
            MachineId machine = data["machine"].get<std::string>();
            int senderPort = data["port"];

            bool foundInstance = false;
            {
                std::lock_guard<std::mutex> lock(mutex);
                logger->info("Call request from {}, call id {}", machine.toString(), id.toString());

                for (auto const& instance : instances.instances()) {
                    if (instance.id() == machine) {
                        incomingCalls.emplace_back(id, instance);
                        auto& newCall = incomingCalls.back();

                        newCall.connect(senderPort);

                        json result;
                        result["id"] = newCall.id().toString();
                        result["port"] = newCall.receiverPort();
                        response.set_content(result.dump(), jsonContentType);
                        foundInstance = true;
                    }
                }
            }

            if (foundInstance) {
                protocol->onNewCall(id);
            } else {
                logger->error("Could not find instance {}", machine.toString());
                response.status = 500;
            }
        });
        httpServer.Post("/call/accept", [this](httplib::Request const& request, httplib::Response& response) {
            auto data = json::parse(request.body);
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

                call->start();
            }

            protocol->onCallAccepted(id);

            json result;
            result["status"] = "ok";
            response.set_content(result.dump(), jsonContentType);
        });
        httpServer.Post("/call/cancel", [this](httplib::Request const& request, httplib::Response& response) {
            auto data = json::parse(request.body);
            UUID id(data["id"].get<std::string>());

            logger->info("Received cancel request for {}", id.toString());
            if (!cancelCall(id)) {
                logger->error("Could not find call {}", id.toString());
                response.status = 500;
                return;
            }

            protocol->onCallCanceled(id);

            json result;
            result["status"] = "ok";
            response.set_content(result.dump(), jsonContentType);
        });
        httpServerThread = std::thread([&]() {
            logger->debug("Start HTTP server on port {}", protocolPort);
            httpServer.listen("0.0.0.0", protocolPort);
        });
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

    bool cancelCall(UUID id, Instance* target = nullptr) {
        std::lock_guard<std::mutex> lock(mutex);

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

    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex);

        auto cleanupCallList = [](std::vector<Call>& calls) {
            calls.erase(std::remove_if(calls.begin(), calls.end(), [](Call const& call) {
                return call.isInvalid();
            }), calls.end());
        };

        cleanupCallList(incomingCalls);
        cleanupCallList(outgoingCalls);
    }

public:
    Logger logger;

    CallProtocol* protocol;

    Instance self;
    InstanceDiscovery const& instances;

    std::mutex mutex;

    // We store incoming and outgoing calls separately, so that we can call
    // ourselves for debugging purposes (in which case we will have both an
    // incoming and an outgoing call with the same id). This wouldn't be
    // necessary when calling other instances.
    std::vector<Call> incomingCalls;
    std::vector<Call> outgoingCalls;

    httplib::Server httpServer;
    std::thread httpServerThread;
};

CallProtocol::CallProtocol(Instance const& self, InstanceDiscovery const& instances) {
    impl = std::make_unique<Impl>(this, self, instances);
}

CallProtocol::~CallProtocol() {}

void CallProtocol::requestCall(Instance const& target) {
    impl->logger->debug("Request call to {}", target.id().toString());

    json data;
    UUID newCallId;
    {
        std::lock_guard<std::mutex> lock(impl->mutex);
        impl->outgoingCalls.emplace_back(target);
        auto& newCall = impl->outgoingCalls.back();

        newCallId = newCall.id();
        impl->logger->debug("New call has id {}", newCallId.toString());
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
    data = json::parse(response->body);

    {
        std::lock_guard<std::mutex> lock(impl->mutex);
        auto call = impl->outgoingCallById(newCallId);
        if (call != nullptr) {
            call->connect(data["port"]);
        }
    }

    onNewCall(newCallId);
}

void CallProtocol::acceptCall(UUID const& id) {
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
        json data;
        data["id"] = id.toString();

        auto request = clientForTarget(*callTarget);
        auto response = request.Post("/call/accept", data.dump(), jsonContentType);
        if (!response || response->status != 200) {
            impl->logger->error("Error while sending request");
            success = false;
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

    onCallAccepted(id);
}

void CallProtocol::cancelCall(UUID const& id) {
    impl->logger->debug("Cancel call {}", id.toString());

    Instance callTarget(getMachineId(), "");
    if (impl->cancelCall(id, &callTarget)) {
        json data;
        data["id"] = id.toString();

        auto request = clientForTarget(callTarget);
        auto response = request.Post("/call/cancel", data.dump(), jsonContentType);
    }

    onCallCanceled(id);
}

std::vector<CallInfo> CallProtocol::currentActiveCalls() const {
    std::lock_guard<std::mutex> lock(impl->mutex);

    std::vector<CallInfo> result;
    auto addCallsFromList = [&result](std::vector<Call> const& calls, bool canBeAccepted) {
        for (auto const& call : calls) {
            if (call.isInvalid()) continue;

            result.push_back({
                call.id(),
                call.target().name(),
                call.isRunning(),
                canBeAccepted && !call.isRunning()
            });
        }
    };

    addCallsFromList(impl->incomingCalls, true);
    addCallsFromList(impl->outgoingCalls, false);

    return result;
}
