#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "instance.h"

class InstanceDiscovery {
public:
    InstanceDiscovery(Instance const& self);
    ~InstanceDiscovery();

    using InstancesChangedCallback = std::function<void(std::vector<Instance> const&)>;
    void onInstancesChanged(InstancesChangedCallback const& callback);

    std::vector<Instance> instances() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
