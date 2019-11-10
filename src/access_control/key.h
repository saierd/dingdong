#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "action.h"
#include "system/uuid.h"
#include "util/json.h"

class Key {
public:
    using Actions = std::vector<std::shared_ptr<Action>>;

    // Hash an unhashed key.
    Key(std::string caption, std::string const& key, Actions actions = {});
    // Create a key with an existing id and hash.
    Key(UUID id, std::string caption, std::string hash, Actions actions = {});

    UUID id() const;
    std::string const& caption() const&;
    Actions const& actions() const&;
    bool hasAction(Action const& action) const;

    void setCaption(std::string newCaption);
    void addAction(std::shared_ptr<Action> const& action);
    void removeAction(Action const& actionToRemove);

    Json toJson() const;
    static Key fromJson(Json const& json, std::function<std::shared_ptr<Action>(std::string const&)> const& actionById);

    bool matches(std::string const& key) const;

private:
    void sortActions();

    // ID of the key, also used as a salt for hashing the key.
    UUID _id;

    std::string _caption;

    // A salted hash of the key.
    std::string _hash;

    // Actions allowed for this key.
    Actions _actions;
};
