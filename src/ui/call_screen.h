#pragma once

#include "screen.h"
#include "call_protocol.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

class CallScreen : public Screen {
public:
    CallScreen();

    Gtk::Widget& widget() override;

    void updateCalls(std::vector<CallInfo> const& calls);

    sigc::signal<void, UUID const&> onAccept;
    sigc::signal<void, UUID const&> onCancel;

private:
    Gtk::Box box;
    Gtk::Button accept, cancel;
    Gtk::Label label;

    UUID currentCall;
};
