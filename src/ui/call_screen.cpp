#include "call_screen.h"

CallScreen::CallScreen() {
    box.pack_start(label);
    box.pack_start(accept);
    box.pack_start(cancel);
    box.show_all();

    accept.set_label("Accept");
    cancel.set_label("Cancel");

    accept.signal_clicked().connect([this]() {
        onAccept(currentCall);
    });
    cancel.signal_clicked().connect([this]() {
        onCancel(currentCall);
    });
}

Gtk::Widget& CallScreen::widget() {
    return box;
}

void CallScreen::updateCalls(std::vector<CallInfo> const& calls) {
    currentCall = calls.back().id;
    label.set_text(calls.back().targetName);
    accept.set_visible(calls.back().canBeAccepted);
}
