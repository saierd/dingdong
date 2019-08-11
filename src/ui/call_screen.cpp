#include "call_screen.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "gtk_helpers.h"

std::string const acceptButtonColor = "#3B3";
std::string const cancelButtonColor = "#E20015";  // Same color as the logo.
std::string const muteButtonColor = "#AAA";
std::string const unmuteButtonColor = "#F88";

int const callButtonSpacing = 20;
int const muteButtonWidth = 300;

class CallWidget {
public:
    explicit CallWidget(CallInfo const& call) : callId(call.id) {
        muted = call.isMuted;

        vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
        vbox.set_spacing(10);
        hbox.set_spacing(10);

        vbox.pack_start(label);
        vbox.pack_start(hbox);
        hbox.set_spacing(callButtonSpacing);
        hbox.pack_start(accept);
        hbox.pack_start(cancel);
        if (call.isRunning) {
            hbox.pack_start(mute, false, true);
            mute.set_size_request(muteButtonWidth);
        }

        loadImageWithSize(acceptIcon, "/call_start.svg", 64, 0, true);
        accept.set_image(acceptIcon);
        styleButton(accept, acceptButtonColor);
        accept.signal_clicked().connect([this]() { onAccept(callId); });

        loadImageWithSize(cancelIcon, "/call_stop.svg", 64, 0, true);
        cancel.set_image(cancelIcon);
        styleButton(cancel, cancelButtonColor);
        cancel.signal_clicked().connect([this]() { onCancel(callId); });

        loadImageWithSize(muteIcon, "/microphone.svg", 64, 0, true);
        loadImageWithSize(unmuteIcon, "/microphone_off.svg", 64, 0, true);
        updateMuteButton();
        mute.signal_clicked().connect([this]() {
            muted = !muted;
            updateMuteButton();
            onMute(callId, muted);
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
    sigc::signal<void, UUID const&, bool> onMute;

private:
    void updateMuteButton() {
        if (muted) {
            styleButton(mute, unmuteButtonColor);
            mute.set_image(unmuteIcon);
        } else {
            styleButton(mute, muteButtonColor);
            mute.set_image(muteIcon);
        }
    }

    UUID callId;
    bool muted = false;

    Gtk::Box vbox, hbox;
    Gtk::Button accept, cancel, mute;
    Gtk::Image acceptIcon, cancelIcon, muteIcon, unmuteIcon;
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
        impl->callWidgets.back().onMute.connect(onMute);

        impl->box.pack_start(impl->callWidgets.back().widget());
    }
}
