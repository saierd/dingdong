#include "key.h"

#include <array>

#include <openssl/md5.h>

#include "system/uuid.h"

std::string hexDump(uint8_t const* data, int size) {
    std::string result;
    result.reserve(2 * size);

    std::array<char, 16> hexCharacters{
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    for (int i = 0; i < size; i++) {
        uint8_t byte = data[i];
        result += hexCharacters[byte >> 4];
        result += hexCharacters[byte & 0xf];
    }

    return result;
}

// Compute a hex dump of the MD5 hash of the given data.
std::string md5Hash(std::string const& data) {
    std::array<uint8_t, MD5_DIGEST_LENGTH> hash;
    MD5(reinterpret_cast<unsigned char const*>(data.data()), data.size(), hash.data());

    return hexDump(hash.data(), MD5_DIGEST_LENGTH);
}

std::string hashKeyWithSalt(std::string const& key, std::string const& salt) {
    return md5Hash(salt + key);
}

Key::Key(std::string caption, std::string const& key, Actions actions)
    : _caption(std::move(caption)), _actions(std::move(actions)) {
    _hash = hashKeyWithSalt(key, _id.toString());
    sortActions();
}

Key::Key(UUID id, std::string caption, std::string hash, Actions actions)
    : _id(id), _caption(std::move(caption)), _hash(std::move(hash)), _actions(std::move(actions)) {
    sortActions();
}

UUID Key::id() const {
    return _id;
}

std::string const& Key::caption() const& {
    return _caption;
}

Key::Actions const& Key::actions() const& {
    return _actions;
}

bool Key::hasAction(Action const& action) const {
    for (auto const& allowedAction : _actions) {
        if (allowedAction->id() == action.id()) {
            return true;
        }
    }
    return false;
}

void Key::setCaption(std::string newCaption) {
    _caption = std::move(newCaption);
}

void Key::addAction(std::shared_ptr<Action> const& action) {
    if (hasAction(*action)) return;
    _actions.push_back(action);
    sortActions();
}

void Key::removeAction(Action const& actionToRemove) {
    _actions.erase(
        std::remove_if(_actions.begin(), _actions.end(),
                       [&actionToRemove](auto const& action) { return action->id() == actionToRemove.id(); }),
        _actions.end());
}

Json Key::toJson() const {
    std::vector<std::string> actions;
    std::transform(_actions.begin(), _actions.end(), std::back_inserter(actions),
                   [](std::shared_ptr<Action> const& action) { return action->id(); });

    return { { "id", _id.toString() }, { "caption", _caption }, { "hash", _hash }, { "actions", actions } };
}

Key Key::fromJson(Json const& json, std::function<std::shared_ptr<Action>(std::string const&)> const& actionById) {
    UUID id(json["id"].get<std::string>());
    std::string caption = json["caption"];
    std::string hash = json["hash"];

    std::vector<std::shared_ptr<Action>> actions;
    for (auto const& actionId : json["actions"]) {
        auto action = actionById(actionId);
        if (action) {
            actions.push_back(action);
        }
    }

    return Key(id, caption, hash, actions);
}

bool Key::matches(std::string const& key) const {
    return _hash == hashKeyWithSalt(key, _id.toString());
}

void Key::sortActions() {
    std::sort(_actions.begin(), _actions.end(), [](std::shared_ptr<Action> const& a, std::shared_ptr<Action> const& b) {
        return a->order() < b->order();
    });
}
