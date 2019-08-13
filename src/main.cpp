#include "call_protocol.h"
#include "discovery.h"
#include "gstreamer/gstreamer.h"
#include "settings.h"
#include "util/logging.h"

#include "ui/call_screen.h"
#include "ui/main_screen.h"
#include "ui/main_window.h"

int main(int argc, char** argv) {
    initializeGStreamer(argc, argv);
    auto app = Gtk::Application::create(argc, argv, "org.dingdong");

    Settings self("settings.json");
    setLogLevel(self.logLevel());

    log()->info("Instance '{}'", self.name());
    log()->info("Machine ID '{}'", self.id().toString());

    InstanceDiscovery discovery(self);

    MainWindow mainWindow;
    MainScreen mainScreen;
    CallScreen callScreen;

    discovery.onInstancesChanged([&mainScreen, &discovery](std::vector<Instance> const&) {
        // Update the buttons on the main screen for the new instances.
        Glib::signal_idle().connect([&]() {
            mainScreen.updateInstances(discovery.instances());
            return false;  // Disconnect the function.
        });
    });

    CallProtocol calls(self, discovery);
    mainScreen.onCall.connect([&calls](Instance const& instance) {
        log()->info("Call {} ({})", instance.id().toString(), instance.name());
        calls.requestCall(instance);
    });

    auto updateCallScreen = [&calls, &mainWindow, &mainScreen, &callScreen, &self]() {
        auto activeCalls = calls.currentActiveCalls();

        if (activeCalls.empty() || !self.showCallScreen()) {
            if (mainWindow.isCurrentScreen(callScreen)) {
                mainWindow.showScreen(mainScreen);
            }
            return;
        }

        callScreen.updateCalls(activeCalls);
        mainWindow.showScreen(callScreen);

        return;
    };

    calls.onCallsChanged.connect([&updateCallScreen]() {
        Glib::signal_idle().connect([&]() {
            updateCallScreen();
            return false;  // Disconnect the function.
        });
    });

    callScreen.onAccept.connect([&calls](UUID const& id) { calls.acceptCall(id); });
    callScreen.onCancel.connect([&calls](UUID const& id) { calls.cancelCall(id); });

    mainWindow.showScreen(mainScreen);
    return app->run(mainWindow);
}
