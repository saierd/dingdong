#include <chrono>
#include <string>

#include <glib.h>

#include <procxx/process.h>

#include "audio_manager.h"
#include "call_history.h"
#include "call_protocol.h"
#include "discovery.h"
#include "gstreamer/gstreamer.h"
#include "pulseaudio_sink_occupier.h"
#include "screen_control.h"
#include "settings.h"
#include "system/beep.h"
#include "system/gpio.h"
#include "util/logging.h"

#include "access_control/access_control.h"
#include "access_control/actions/callback.h"
#include "access_control/rfid_scanner.h"

#include "ui/main_window.h"
#include "ui/screens/action_screen.h"
#include "ui/screens/call_history_screen.h"
#include "ui/screens/call_screen.h"
#include "ui/screens/key_input_screen.h"
#include "ui/screens/key_screen.h"
#include "ui/screens/main_screen.h"
#include "ui/x_helpers.h"

std::string const settingsFile = "settings/settings.json";
std::string const keyFile = "settings/keys.json";

std::string const shutdownAction = "shutdown";
std::string const shutdownActionCaption = "Shutdown Application";
std::string const manageKeysAction = "manage_keys";
std::string const manageKeysActionCaption = "Manage Keys";

std::chrono::seconds const checkScreenInterval(1);
std::chrono::milliseconds const checkMotionSensorInterval(200);
std::chrono::milliseconds const checkRingButtonInterval(100);

int main(int argc, char** argv) {
    initializeGStreamer(argc, argv);

    // Occupy the PulseAudio sink as a workaround for weird PulseAudio behavior. See the description of
    // PulseAudioSinkOccupier for more information.
    PulseAudioSinkOccupier sinkOccupier;

    auto app = Gtk::Application::create(argc, argv, "org.dingdong");

    Settings self(settingsFile);
    setLogLevel(self.logLevel());

    log()->info("Instance '{}'", self.name());
    log()->info("Machine ID '{}'", self.id().toString());

    std::function<void()> openKeyScreen;

    AccessControl accessControl;
    accessControl.addAction(std::make_unique<CallbackAction>(shutdownAction, shutdownActionCaption, [&app]() {
#ifdef RASPBERRY_PI
        // Stop the service to avoid restarting immediately. The service will automatically be started again when the
        // system reboots.
        try {
            procxx::process("sudo", "service", "dingdong", "stop").exec();
        } catch (...) {
        }
#endif
        app->quit();
    }));
    accessControl.addAction(std::make_unique<CallbackAction>(manageKeysAction, manageKeysActionCaption,
                                                             [&openKeyScreen]() { openKeyScreen(); }));
    accessControl.addActionsFromJson(self.actions());
    accessControl.loadKeyFile(keyFile);

    // Add remote executable actions to the self instance so that we advertise them in the discovery.
    for (auto const& action : accessControl.actions()) {
        if (!action->allowRemoteExecution()) continue;
        self.addRemoteAction({ action->id(), action->caption() });
    }

    InstanceDiscovery discovery(self);
    auto audioManager = std::make_shared<AudioManager>(self);

    ActionScreen actionScreen;
    CallHistoryScreen callHistoryScreen;
    CallScreen callScreen;
    KeyScreen keyScreen(&accessControl);
    KeyInputScreen keyInputScreen(false);
    MainScreen mainScreen;

    MainWindow mainWindow(mainScreen);

    openKeyScreen = [&mainWindow, &keyScreen]() { mainWindow.pushScreen(keyScreen); };

    // On first launch, show a button for accessing the key settings to allow setting up an administration key.
    ScreenButton manageKeysButton("", openKeyScreen);
    manageKeysButton.icon = "/settings.svg";
    mainWindow.addPermanentButton(manageKeysButton, [&accessControl]() {
        // Hide the button when there is already a key which can manage keys.
        bool hasAdminKey = false;
        for (auto const& key : accessControl.keys()) {
            for (auto const& action : key.actions()) {
                if (action->id() == manageKeysAction) {
                    hasAdminKey = true;
                    break;
                }
            }
        }

        return !hasAdminKey;
    });

    ScreenButton inputKeyButton("", [&mainWindow, &keyInputScreen]() { mainWindow.pushScreen(keyInputScreen); });
    inputKeyButton.icon = "/key.svg";
    mainWindow.addPermanentButton(inputKeyButton, [&mainWindow, &mainScreen]() {
        // Allow on main screen only.
        return mainWindow.isCurrentScreen(mainScreen);
    });

    keyInputScreen.onKeyInput.connect([&accessControl, &mainWindow, &actionScreen](std::string const& inputKey) {
        for (auto const& key : accessControl.keys()) {
            if (key.matches(inputKey)) {
                // Pop the key input screen.
                mainWindow.popScreen();

                log()->info("Key input '{}'", key.caption());

                if (key.actions().size() == 1) {
                    key.actions()[0]->trigger();
                } else if (key.actions().size() > 1) {
                    actionScreen.updateActions(key.actions());
                    mainWindow.pushScreen(actionScreen);
                }
                break;
            }
        }
    });

#ifdef RASPBERRY_PI
    RfidScanner rfidScanner;
    rfidScanner.onKeyScanned.connect([&mainWindow, &accessControl, &mainScreen, &actionScreen](std::string scannedKey) {
        Glib::signal_idle().connect([&, scannedKey = std::move(scannedKey)]() {
            if (mainWindow.handleScannedKey(scannedKey)) {
                return false;  // Disconnect the function.
            }

            beep();

            for (auto const& key : accessControl.keys()) {
                if (key.matches(scannedKey)) {
                    log()->info("Scanned key '{}'", key.caption());

                    if (key.actions().size() == 1) {
                        key.actions()[0]->trigger();
                    } else if (key.actions().size() > 1 && mainWindow.isCurrentScreen(mainScreen)) {
                        actionScreen.updateActions(key.actions());
                        mainWindow.pushScreen(actionScreen);
                    }
                    break;
                }
            }

            return false;  // Disconnect the function.
        });
    });
#endif

    CallHistory incomingCallHistory;
    CallProtocol calls(self, &incomingCallHistory, discovery, audioManager);
    mainScreen.onCall.connect([&calls](Instance const& instance) {
        log()->info("Call {} ({})", instance.id().toString(), instance.name());
        calls.requestCall(instance);
    });

    auto updateCallScreen = [&calls, &mainWindow, &mainScreen, &callScreen, &self]() {
        auto activeCalls = calls.currentActiveCalls();

        if (activeCalls.empty() || !self.showCallScreen()) {
            if (mainWindow.isCurrentScreen(callScreen)) {
                mainWindow.pushScreen(mainScreen);
            }
            return;
        }

        callScreen.updateCalls(activeCalls);
        mainWindow.pushScreen(callScreen);
    };

    calls.onCallsChanged.connect([&updateCallScreen]() {
        Glib::signal_idle().connect([&]() {
            updateCallScreen();
            return false;  // Disconnect the function.
        });
    });

    calls.onActionRequested.connect([&](std::string const& actionId) {
        for (auto const& action : accessControl.actions()) {
            if (action->id() != actionId || !action->allowRemoteExecution()) continue;

            action->trigger();
            break;
        }
    });

    callScreen.onAccept.connect([&calls](UUID const& id) { calls.acceptCall(id); });
    callScreen.onCancel.connect([&calls](UUID const& id) { calls.cancelCall(id); });
    callScreen.onMute.connect([&calls](UUID const& id, bool mute) { calls.muteCall(id, mute); });
    callScreen.onEnableVideo.connect([&calls](UUID const& id, bool enable) { calls.enableVideoForCall(id, enable); });
    callScreen.onRequestAction.connect(
        [&calls](UUID const& callId, std::string const& actionId) { calls.requestRemoteAction(callId, actionId); });

    auto updateInstanceScreen = [&mainScreen, &discovery, &calls]() {
        // Update the buttons on the main screen for the new instances.
        Glib::signal_idle().connect([&]() {
            mainScreen.updateInstances(discovery.instances(), calls.currentActiveCalls());
            return false;  // Disconnect the function.
        });
    };

    discovery.onInstancesChanged(updateInstanceScreen);
    calls.onCallsChanged.connect(updateInstanceScreen);

    ScreenButton callHistoryButton("",
                                   [&mainWindow, &callHistoryScreen]() { mainWindow.pushScreen(callHistoryScreen); });
    callHistoryButton.icon = "/call_missed.svg";
    callHistoryButton.color = colorRed;
    mainWindow.addPermanentButton(callHistoryButton, [&mainWindow, &mainScreen, &incomingCallHistory]() {
        // Allow on main screen only.
        if (!mainWindow.isCurrentScreen(mainScreen)) {
            return false;
        }

        return incomingCallHistory.numMissedEntries() > 0;
    });

    auto updateCallHistoryScreen = [&callHistoryScreen, &incomingCallHistory, &mainWindow]() {
        Glib::signal_idle().connect([&]() {
            callHistoryScreen.updateHistory(incomingCallHistory);
            mainWindow.updateButtons();
            return false;  // Disconnect the function.
        });
    };

    updateCallHistoryScreen();
    incomingCallHistory.onEntriesChanged.connect(updateCallHistoryScreen);
    callHistoryScreen.onClear.connect([&incomingCallHistory]() { incomingCallHistory.clear(); });

    // Forces the screen to be on unless we are on the main screen.
    auto turnOnScreenIfNecessary = [&mainWindow, &mainScreen]() {
        if (!mainWindow.isCurrentScreen(mainScreen)) {
            turnScreenOn();
        }
    };

    // Permanently check whether we currently show the main screen. If not, force the screen to be enabled.
    Glib::signal_timeout().connect(
        [&turnOnScreenIfNecessary]() -> bool {
            turnOnScreenIfNecessary();
            return true;
        },
        std::chrono::duration_cast<std::chrono::milliseconds>(checkScreenInterval).count());

    // Enable the screen immediately when the screen switched to a non-main screen (e.g. because we received a call or
    // an RFID tag was scanned).
    mainWindow.onScreenChanged.connect(turnOnScreenIfNecessary);

    if (self.motionSensorPin() >= 0) {
        // Check the motion sensor periodically and turn on the screen if it is active.
        Glib::signal_timeout().connect(
            [motionSensorPin = GpioInputPin(self.motionSensorPin())]() -> bool {
                if (motionSensorPin.read()) {
                    turnScreenOn();
                }
                return true;
            },
            std::chrono::duration_cast<std::chrono::milliseconds>(checkMotionSensorInterval).count());
    }

    if (self.ringButtonPin() >= 0) {
        // Check the ring button periodically and play a ringtone if it becomes active.
        Glib::signal_timeout().connect(
            [ringButtonPin = GpioInputPin(self.ringButtonPin(), true), buttonPressedLastTime = false, audioManager,
             ringtone = self.ringButtonRingtone()]() mutable -> bool {
                bool buttonPressed = !ringButtonPin.read();
                if (!buttonPressedLastTime && buttonPressed) {
                    audioManager->playRingtone(ringtone);
                }
                buttonPressedLastTime = buttonPressed;
                return true;
            },
            std::chrono::duration_cast<std::chrono::milliseconds>(checkRingButtonInterval).count());
    }

#ifdef RASPBERRY_PI
    // Hide the cursor on the touch screen.
    hideCursorForApplication();
#endif

    mainWindow.pushScreen(mainScreen);
    return app->run(mainWindow);
}
