#pragma once

#include <string>

class Action {
public:
    virtual ~Action() = default;

    virtual std::string id() const = 0;
    virtual std::string caption() const = 0;
    virtual void trigger() const = 0;

    virtual bool allowRemoteExecution() const {
        return false;
    }
};
