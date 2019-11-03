#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <glibmm.h>

#include "discovery.h"
#include "instance.h"
#include "settings.h"
#include "system/uuid.h"

struct CallInfo {
    UUID id;
    MachineId targetId;
    std::string targetName;
    bool isRunning;
    bool isMuted;
    bool canBeAccepted;

    std::vector<Instance::RemoteAction> remoteActions;
};

class CallProtocol {
public:
    CallProtocol(Settings const& self, InstanceDiscovery const& instances);
    ~CallProtocol();

    void requestCall(Instance const& target);

    // Accept an incoming call.
    void acceptCall(UUID const& id, std::optional<int> receiverPort = std::nullopt);
    // Reject an incoming call or stop any active call with the given id.
    void cancelCall(UUID const& id);
    void muteCall(UUID const& id, bool mute = true);

    void requestRemoteAction(UUID const& callId, std::string const& actionId);

    sigc::signal<void> onCallsChanged;
    std::vector<CallInfo> currentActiveCalls() const;

    sigc::signal<void, std::string> onActionRequested;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
