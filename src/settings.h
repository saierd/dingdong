#pragma once

#include <memory>
#include <string>
#include <vector>

#include "instance.h"
#include "util/json.h"

class Settings : public Instance {
public:
    explicit Settings(std::string const& settingsFile);

    bool showCallScreen() const {
        return _showCallScreen;
    }

    bool autoAccept() const {
        return _autoAccept;
    }

    std::string ringtone() const {
        return _ringtone;
    }

    int motionSensorPin() const {
        return _motionSensorPin;
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
    int _motionSensorPin = -1;

    std::string _logLevel;

    Json _actions;
};
