#pragma once

#include <string>
#include <vector>

#include "network/ip_address.h"
#include "system/machine_id.h"

// An instance of the application running on the network.
class Instance {
public:
    struct RemoteAction {
        std::string id;
        std::string caption;

        bool operator==(RemoteAction const& other) const {
            return id == other.id && caption == other.caption;
        }

        bool operator!=(RemoteAction const& other) const {
            return !(*this == other);
        }
    };

public:
    Instance(MachineId id, std::string name, IpAddress ipAddress = IpAddress(), int order = 0,
             bool canReceiveVideo = true, std::vector<RemoteAction> remoteActions = {})
        : _id(id),
          _name(std::move(name)),
          _ipAddress(ipAddress),
          _order(order),
          _canReceiveVideo(canReceiveVideo),
          _remoteActions(std::move(remoteActions)) {}

    MachineId const& id() const& {
        return _id;
    }

    std::string const& name() const& {
        return _name;
    }

    IpAddress const& ipAddress() const& {
        return _ipAddress;
    }

    int order() const {
        return _order;
    }

    bool canReceiveVideo() const {
        return _canReceiveVideo;
    }

    void addRemoteAction(RemoteAction action) {
        _remoteActions.emplace_back(std::move(action));
    }

    std::vector<RemoteAction> const& remoteActions() const& {
        return _remoteActions;
    }

    bool operator==(Instance const& other) const {
        return _id == other._id && _name == other._name && _ipAddress == other._ipAddress && _order == other._order &&
               _canReceiveVideo == other._canReceiveVideo && _remoteActions == other._remoteActions;
    }

    bool operator!=(Instance const& other) const {
        return !(*this == other);
    }

protected:
    MachineId _id;
    std::string _name;
    IpAddress _ipAddress;
    int _order;
    bool _canReceiveVideo;

    std::vector<RemoteAction> _remoteActions;
};
