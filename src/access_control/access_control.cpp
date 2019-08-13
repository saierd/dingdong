#include "access_control.h"

#include <algorithm>
#include <fstream>
#include <iomanip>

#include "actions/gpio.h"

std::vector<std::shared_ptr<Action>> const& AccessControl::actions() const& {
    return _actions;
}

std::shared_ptr<Action> AccessControl::actionById(std::string const& id) const {
    for (auto const& action : _actions) {
        if (action->id() == id) {
            return action;
        }
    }
    return nullptr;
}

void AccessControl::addAction(std::shared_ptr<Action> action) {
    _actions.emplace_back(std::move(action));
}

void AccessControl::addActionsFromJson(Json const& json) {
    if (!json.is_object()) return;

    for (auto const& element : json.items()) {
        auto action = GpioAction::fromJson(element.key(), element.value());
        if (action) {
            _actions.emplace_back(std::move(action));
        }
    }
}

std::vector<Key> const& AccessControl::keys() const& {
    return _keys;
}

void AccessControl::insertOrReplaceKey(Key key) {
    for (auto& existingKey : _keys) {
        if (existingKey.id() == key.id()) {
            existingKey = key;
            return;
        }
    }

    _keys.emplace_back(std::move(key));
}

void AccessControl::removeKey(Key const& keyToRemove) {
    _keys.erase(std::remove_if(_keys.begin(), _keys.end(),
                               [&keyToRemove](auto const& key) { return key.id() == keyToRemove.id(); }),
                _keys.end());
}

void AccessControl::loadKeyFile(std::string const& keyFile) {
    loadedKeyFile = keyFile;
    reloadKeyFile();
}

void AccessControl::reloadKeyFile() {
    _keys.clear();
    if (loadedKeyFile.empty()) return;

    std::ifstream file(loadedKeyFile);
    if (file) {
        auto data = Json::parse(file);

        for (auto const& keyData : data) {
            _keys.emplace_back(Key::fromJson(keyData, [this](std::string const& id) { return actionById(id); }));
        }
    }
}

void AccessControl::saveKeyFile() const {
    if (loadedKeyFile.empty()) return;

    Json data;
    for (auto const& key : _keys) {
        data.push_back(key.toJson());
    }

    std::ofstream file(loadedKeyFile);
    file << std::setw(4) << data << std::endl;
}
