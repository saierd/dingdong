#pragma once

#include <string>

class Action {
public:
    virtual ~Action() = default;

    virtual std::string id() const = 0;
    virtual std::string caption() const = 0;
    virtual void trigger() const = 0;

    virtual int order() const {
        return 0;
    }

    virtual bool allowRemoteExecution() const {
        return false;
    }

    // When a user triggers this action and this method returns true, the action screen will stay visible for a short
    // time and allow the user to trigger additional actions.
    virtual bool allowAdditionalAction() const {
        return false;
    }
};
