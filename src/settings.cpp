#include "settings.h"

#include <fstream>

template<typename T>
void getValueFromJson(T& variable, Json const& data, std::string const& key) {
    if (data.count(key) > 0) {
        variable = data[key];
    }
}

Settings::Settings(std::string const& settingsFile) : Instance(getMachineId(), "Unknown Instance") {
    std::ifstream file(settingsFile);
    if (file) {
        auto data = Json::parse(file);

        getValueFromJson(_name, data, "instanceName");
        getValueFromJson(_order, data, "instanceOrder");
        getValueFromJson(_showCallScreen, data, "showCallScreen");
        getValueFromJson(_autoAccept, data, "autoAccept");
        if (!_showCallScreen) {
            _autoAccept = true;  // Can't accept calls otherwise.
        }
        getValueFromJson(_ringtone, data, "ringtone");
        getValueFromJson(_ringtoneVolume, data, "ringtoneVolume");
        if (data.count("ringButton") > 0) {
            getValueFromJson(_ringButtonPin, data["ringButton"], "pin");
            getValueFromJson(_ringButtonRingtone, data["ringButton"], "ringtone");
        }
        getValueFromJson(_callVolume, data, "callVolume");
        getValueFromJson(_motionSensorPin, data, "motionSensorPin");

        getValueFromJson(_logLevel, data, "logLevel");

        getValueFromJson(_actions, data, "actions");

        _canReceiveVideo = _showCallScreen;
    }
}
