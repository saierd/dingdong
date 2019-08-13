#include "callback.h"

#include "util/logging.h"

std::string const actionLoggingCategory = "action";

CallbackAction::CallbackAction(std::string id, std::string caption, Callback callback)
    : _id(std::move(id)), _caption(std::move(caption)), _callback(std::move(callback)) {}

std::string CallbackAction::id() const {
    return _id;
}

std::string CallbackAction::caption() const {
    return _caption;
}

void CallbackAction::trigger() const {
    categoryLogger(actionLoggingCategory)->info("Trigger action '{}'", id());
    _callback();
}
