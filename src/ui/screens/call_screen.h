#pragma once

#include <memory>

#include "call_protocol.h"
#include "ui/screen.h"

class CallScreen : public Screen {
public:
    CallScreen();
    ~CallScreen() override;

    Gtk::Widget& widget() override;

    void updateCalls(std::vector<CallInfo> const& calls);

    sigc::signal<void, UUID const&> onAccept;
    sigc::signal<void, UUID const&> onCancel;
    sigc::signal<void, UUID const&, bool> onMute;
    sigc::signal<void, UUID const&, std::string const&> onRequestAction;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
