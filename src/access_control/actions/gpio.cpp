#include "gpio.h"

#include "system/gpio.h"
#include "util/logging.h"

std::string const actionLoggingCategory = "action";

class GpioAction::Impl {
public:
    std::string id;
    std::string caption;
    bool allowRemoteExecution = false;

    std::unique_ptr<GpioOutputPin> pin;
    std::chrono::milliseconds duration;

    std::thread thread;

    void joinThread() {
        if (thread.joinable()) {
            thread.join();
        }
    }
};

GpioAction::GpioAction() {
    impl = std::make_unique<Impl>();
}

GpioAction::~GpioAction() {
    impl->joinThread();
}

std::shared_ptr<GpioAction> GpioAction::fromJson(std::string id, Json const& json) {
    auto action = std::make_shared<GpioAction>();

    action->impl->id = std::move(id);
    action->impl->caption = json["caption"];
    if (json.find("remote") != json.end()) {
        action->impl->allowRemoteExecution = json["remote"].get<bool>();
    }

    if (action->impl->id.empty() || action->impl->caption.empty()) return nullptr;

    unsigned int pin = json["pin"];
    action->impl->pin = std::make_unique<GpioOutputPin>(pin);

    action->impl->duration = std::chrono::milliseconds(json["duration"].get<unsigned int>());

    return action;
}

std::string GpioAction::id() const {
    return impl->id;
}

std::string GpioAction::caption() const {
    return impl->caption;
}

void GpioAction::trigger() const {
    if (!impl->pin) return;

    categoryLogger(actionLoggingCategory)->info("Trigger action '{}'", id());

    impl->joinThread();
    impl->thread = std::thread([this]() {
        impl->pin->set(true);
        std::this_thread::sleep_for(impl->duration);
        impl->pin->set(false);
    });
}

bool GpioAction::allowRemoteExecution() const {
    return impl->allowRemoteExecution;
}
