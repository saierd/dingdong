#pragma once

#include "glibmm.h"

#include "discovery.h"
#include "instance.h"
#include "settings.h"
#include "system/uuid.h"

struct CallInfo {
    UUID id;
    std::string targetName;
    bool isRunning;
    bool canBeAccepted;
};

class CallProtocol {
public:
    CallProtocol(Settings const& self, InstanceDiscovery const& instances);
    ~CallProtocol();

    void requestCall(Instance const& target);

    // Accept an incoming call.
    void acceptCall(UUID const& id);
    // Reject an incoming call or stop any active call with the given id.
    void cancelCall(UUID const& id);

    sigc::signal<void> onCallsChanged;
    std::vector<CallInfo> currentActiveCalls() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
