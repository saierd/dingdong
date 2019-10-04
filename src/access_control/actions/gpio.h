#pragma once

#include <memory>
#include <string>

#include "access_control/action.h"
#include "util/json.h"

class GpioAction : public Action {
public:
    GpioAction();
    ~GpioAction() override;

    static std::shared_ptr<GpioAction> fromJson(std::string id, Json const& json);

    std::string id() const override;
    std::string caption() const override;
    void trigger() const override;

    bool allowRemoteExecution() const override;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
