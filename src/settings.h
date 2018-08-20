#pragma once

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

private:
    bool _showCallScreen = true;
    bool _autoAccept = false;
};
