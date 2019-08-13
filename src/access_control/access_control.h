#pragma once

#include <memory>
#include <vector>

#include "action.h"
#include "key.h"
#include "util/json.h"

class AccessControl {
public:
    std::vector<std::shared_ptr<Action>> const& actions() const&;
    std::shared_ptr<Action> actionById(std::string const& id) const;
    void addAction(std::shared_ptr<Action> action);
    void addActionsFromJson(Json const& json);

    std::vector<Key> const& keys() const&;
    void insertOrReplaceKey(Key key);
    void removeKey(Key const& keyToRemove);

    void loadKeyFile(std::string const& keyFile);
    void reloadKeyFile();
    void saveKeyFile() const;

private:
    std::string loadedKeyFile;

    std::vector<std::shared_ptr<Action>> _actions;
    std::vector<Key> _keys;
};
