#pragma once

#include <functional>
#include <string>

#include "access_control/action.h"

class CallbackAction : public Action {
public:
    using Callback = std::function<void()>;

    CallbackAction(std::string id, std::string caption, Callback callback);

    std::string id() const override;
    std::string caption() const override;
    void trigger() const override;

private:
    std::string _id;
    std::string _caption;
    Callback _callback;
};
