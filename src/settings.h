#pragma once

#include <string>

#include "instance.h"

class Settings : public Instance {
public:
    Settings(std::string settingsFile);

    bool showCallScreen() const {
        return _showCallScreen;
    }

    bool autoAccept() const {
        return _autoAccept;
    }

    std::string audioSourceDevice() const {
        return _audioSourceDevice;
    }

    std::string ringtone() const {
        return _ringtone;
    }

private:
    bool _showCallScreen = true;
    bool _autoAccept = false;
    std::string _audioSourceDevice;
    std::string _ringtone;
};
