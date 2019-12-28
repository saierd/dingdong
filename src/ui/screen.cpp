#include "screen.h"

#include <gtkmm.h>

#include "main_window.h"

void BaseScreen::notifyShown() {
    onShow();
}

void BaseScreen::notifyPopped() {
    if (timeoutConnection.connected()) {
        timeoutConnection.disconnect();
    }

    onPop();
}

void BaseScreen::setTimeout(std::chrono::milliseconds duration) {
    timeoutConnection = Glib::signal_timeout().connect(
        [this]() -> bool {
            if (mainWindow && mainWindow->isCurrentScreen(*this)) {
                mainWindow->popScreen();
            }
            return false;
        },
        duration.count());
}
