#include <glibmm/dispatcher.h>

#include "call_protocol.h"
#include "discovery.h"
#include "settings.h"
#include "stream/gstreamer.h"
#include "util/logging.h"

#include "ui/call_screen.h"
#include "ui/main_window.h"
#include "ui/main_screen.h"

int main(int argc, char** argv) {
    initializeGStreamer(argc, argv);
    auto app = Gtk::Application::create(argc, argv, "org.dingdong");

    Settings self("Test Instance");
    log()->info("Instance {}", self.name());
    log()->info("Machine ID {}", self.id().toString());

    InstanceDiscovery discovery(self);

    MainWindow mainWindow;
    MainScreen mainScreen;
    CallScreen callScreen;

    Glib::Dispatcher instancesChangedSignal;

    instancesChangedSignal.connect([&discovery, &mainScreen]() {
        mainScreen.updateInstances(discovery.instances());
    });
    discovery.onInstancesChanged([&instancesChangedSignal](std::vector<Instance> const&) {
        instancesChangedSignal();
    });

    CallProtocol calls(self, discovery);
    mainScreen.onCall.connect([&calls](Instance const& instance) {
        log()->info("Call {} ({})", instance.id().toString(), instance.name());
        calls.requestCall(instance);
    });

    auto updateCallScreen = [&calls, &mainWindow, &mainScreen, &callScreen]() {
        auto activeCalls = calls.currentActiveCalls();

        if (activeCalls.empty()) {
            if (mainWindow.isCurrentScreen(callScreen)) {
                mainWindow.showScreen(mainScreen);
            }
            return;
        }

        callScreen.updateCalls(activeCalls);
        mainWindow.showScreen(callScreen);

        return;
    };

    auto updateCallScreenWhenIdle = [&]() {
        Glib::signal_idle().connect([&]() {
            updateCallScreen();
            return false; // Disconnect the function.
        });
    };

    calls.onNewCall.connect([&updateCallScreenWhenIdle](UUID const&) {
        updateCallScreenWhenIdle();
    });
    calls.onCallAccepted.connect([&updateCallScreenWhenIdle](UUID const&) {
        updateCallScreenWhenIdle();
    });
    calls.onCallCanceled.connect([&updateCallScreenWhenIdle](UUID const&) {
        updateCallScreenWhenIdle();
    });

    callScreen.onAccept.connect([&calls](UUID const& id) {
        calls.acceptCall(id);
    });
    callScreen.onCancel.connect([&calls](UUID const& id) {
        calls.cancelCall(id);
    });

    mainWindow.showScreen(mainScreen);
    return app->run(mainWindow);
}
