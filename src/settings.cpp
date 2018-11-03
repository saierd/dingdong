#include "settings.h"

#include <fstream>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

template<typename T>
void getValueFromJson(T& variable, json const& data, std::string const& key) {
    if (data.count(key) > 0) {
        variable = data[key];
    }
}

Settings::Settings(std::string settingsFile) : Instance(getMachineId(), "Unknown Instance") {
    std::ifstream file(settingsFile);
    if (file) {
        auto data = json::parse(file);

        getValueFromJson(_name, data, "instanceName");
        getValueFromJson(_showCallScreen, data, "showCallScreen");
        getValueFromJson(_autoAccept, data, "autoAccept");
        if (!_showCallScreen) {
            _autoAccept = true; // Can't accept calls otherwise.
        }
        getValueFromJson(_audioSourceDevice, data, "audioSourceDevice");
        getValueFromJson(_ringtone, data, "ringtone");
    }
}
