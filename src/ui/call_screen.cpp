#include "call_screen.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

#include "gtk_helpers.h"

class CallWidget {
public:
    CallWidget(CallInfo const& call) : callId(call.id) {
        vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
        vbox.set_spacing(10);
        hbox.set_spacing(10);

        vbox.pack_start(label);
        vbox.pack_start(hbox);
        hbox.pack_start(accept);
        hbox.pack_start(cancel);

        accept.set_label("Accept");
        applyCss(accept, "* { background: #3B3; }");
        accept.signal_clicked().connect([this]() {
            onAccept(callId);
        });

        cancel.set_label("Cancel");
        applyCss(cancel, "* { background: #E22; }");
        cancel.signal_clicked().connect([this]() {
            onCancel(callId);
        });

        label.set_text(call.targetName);

        vbox.show_all();
        accept.set_visible(call.canBeAccepted);
    }

    Gtk::Widget& widget() {
        return vbox;
    }

    sigc::signal<void, UUID const&> onAccept;
    sigc::signal<void, UUID const&> onCancel;

private:
    UUID callId;

    Gtk::Box vbox, hbox;
    Gtk::Button accept, cancel;
    Gtk::Label label;
};

class CallScreen::Impl {
public:
    Gtk::Box box;
    std::vector<CallWidget> callWidgets;
};

CallScreen::CallScreen() {
    impl = std::make_unique<Impl>();
    impl->box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    impl->box.show();
}

CallScreen::~CallScreen() {}

Gtk::Widget& CallScreen::widget() {
    return impl->box;
}

void CallScreen::updateCalls(std::vector<CallInfo> const& calls) {
    impl->callWidgets.clear();
    for (auto const& call : calls) {
        impl->callWidgets.emplace_back(call);
        impl->callWidgets.back().onAccept.connect(onAccept);
        impl->callWidgets.back().onCancel.connect(onCancel);

        impl->box.pack_start(impl->callWidgets.back().widget());
    }
}
