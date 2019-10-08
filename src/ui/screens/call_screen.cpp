#include "call_screen.h"

#include <gdk/gdkx.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "stream/video.h"

#include "ui/constants.h"
#include "ui/gtk_helpers.h"

// typedef struct _GdkWindow GdkWindow;
// unsigned long gdk_x11_window_get_xid(GdkWindow* window);

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
        sendingVideo = call.sendingVideo;

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

            hbox.pack_start(video, false, true);
            video.set_size_request(muteButtonWidth);
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
        updateVideoButton();
        video.signal_clicked().connect([this]() {
            sendingVideo = !sendingVideo;
            updateVideoButton();
            onEnableVideo(callId, sendingVideo);
        });

        setFont(label, largeFontSize, true);
        label.set_text(call.targetName);

        vbox.pack_start(actionsHBox);
        actionsHBox.set_spacing(callButtonSpacing);
        for (auto const& remoteAction : call.remoteActions) {
            actionButtons.emplace_back(remoteAction.caption);
            styleButton(actionButtons.back());
            setFont(actionButtons.back(), mediumFontSize);
            actionButtons.back().signal_clicked().connect(
                [this, callId = call.id, actionId = remoteAction.id]() { onRequestAction(callId, actionId); });

            actionsHBox.pack_start(actionButtons.back());
        }

        if (call.remoteSendsVideo) {
            vbox.pack_start(videoArea, true, true);
            videoArea.signal_realize().connect([videoReceiver = call.videoReceiver, &area = videoArea]() {
                guintptr xid = gdk_x11_window_get_xid(area.get_window()->gobj());
                videoReceiver->setWindowHandle(xid);
            });
        }

        vbox.show_all();
        accept.set_visible(call.canBeAccepted);
    }

    Gtk::Widget& widget() {
        return vbox;
    }

    sigc::signal<void, UUID const&> onAccept;
    sigc::signal<void, UUID const&> onCancel;
    sigc::signal<void, UUID const&, bool> onMute;
    sigc::signal<void, UUID const&, bool> onEnableVideo;
    sigc::signal<void, UUID const&, std::string const&> onRequestAction;

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

    void updateVideoButton() {
        styleButton(video);

        if (sendingVideo) {
            video.set_label("SENDING VIDEO");
        } else {
            video.set_label("NOT SENDING VIDEO");
        }
    }

    UUID callId;
    bool muted = false;
    bool sendingVideo = false;

    Gtk::Box vbox, hbox, actionsHBox;
    Gtk::Button accept, cancel, mute, video;
    Gtk::Image acceptIcon, cancelIcon, muteIcon, unmuteIcon;
    Gtk::Label label;

    Gtk::DrawingArea videoArea;

    std::vector<Gtk::Button> actionButtons;
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
        impl->callWidgets.back().onEnableVideo.connect(onEnableVideo);
        impl->callWidgets.back().onRequestAction.connect(onRequestAction);

        if (widget().get_parent() && call.remoteSendsVideo) {
            onVideoWidgetAddedForCall(call.id);
        }

        impl->box.pack_start(impl->callWidgets.back().widget());
    }
}
