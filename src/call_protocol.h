#pragma once

#include "glibmm.h"

#include "discovery.h"
#include "instance.h"
#include "system/uuid.h"

struct CallInfo {
    UUID id;
    std::string targetName;
    bool isRunning;
    bool canBeAccepted;
};

class CallProtocol {
public:
    CallProtocol(Instance const& self, InstanceDiscovery const& instances);
    ~CallProtocol();

    void requestCall(Instance const& target);

    void acceptCall(UUID const& id);
    void cancelCall(UUID const& id);

    std::vector<CallInfo> currentActiveCalls() const;

    sigc::signal<void, UUID> onNewCall;
    sigc::signal<void, UUID> onCallAccepted;
    sigc::signal<void, UUID> onCallCanceled;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
