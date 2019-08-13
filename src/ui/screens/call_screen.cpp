#include "call_screen.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "ui/constants.h"
#include "ui/gtk_helpers.h"

std::string const acceptButtonColor = colorGreen;
std::string const cancelButtonColor = colorRed;
std::string const muteButtonColor = colorLightGrey;
std::string const unmuteButtonColor = colorLightRed;

int const callButtonSpacing = defaultSpacing;
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

        loadButtonIconLarge(acceptIcon, "/call_start.svg");
        accept.set_image(acceptIcon);
        styleButton(accept, acceptButtonColor, largePadding);
        accept.signal_clicked().connect([this]() { onAccept(callId); });

        loadButtonIconLarge(cancelIcon, "/call_stop.svg");
        cancel.set_image(cancelIcon);
        styleButton(cancel, cancelButtonColor, largePadding);
        cancel.signal_clicked().connect([this]() { onCancel(callId); });

        loadButtonIconLarge(muteIcon, "/microphone.svg");
        loadButtonIconLarge(unmuteIcon, "/microphone_off.svg");
        updateMuteButton();
        mute.signal_clicked().connect([this]() {
            muted = !muted;
            updateMuteButton();
            onMute(callId, muted);
        });

        setFont(label, largeFontSize, true);
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
            styleButton(mute, unmuteButtonColor, largePadding);
            mute.set_image(unmuteIcon);
        } else {
            styleButton(mute, muteButtonColor, largePadding);
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

CallScreen::~CallScreen() = default;

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
