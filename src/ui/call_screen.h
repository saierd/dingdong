#pragma once

#include <memory>

#include "screen.h"
#include "call_protocol.h"

class CallScreen : public Screen {
public:
    CallScreen();
    ~CallScreen();

    Gtk::Widget& widget() override;

    void updateCalls(std::vector<CallInfo> const& calls);

    sigc::signal<void, UUID const&> onAccept;
    sigc::signal<void, UUID const&> onCancel;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
