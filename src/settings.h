#pragma once

#include <memory>
#include <string>
#include <vector>

#include "instance.h"
#include "util/json.h"

class Settings : public Instance {
public:
    explicit Settings(std::string settingsFile);

    bool showCallScreen() const {
        return _showCallScreen;
    }

    bool autoAccept() const {
        return _autoAccept;
    }

    std::string ringtone() const {
        return _ringtone;
    }

    std::string logLevel() const {
        return _logLevel;
    }

    Json actions() const {
        return _actions;
    }

private:
    bool _showCallScreen = true;
    bool _autoAccept = false;
    std::string _ringtone;

    std::string _logLevel;

    Json _actions;
};
