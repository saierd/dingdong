#include "screen.h"

#include <gtkmm.h>

#include "main_window.h"

void Screen::notifyShown() {
    onShow();
}

void Screen::notifyPopped() {
    if (timeoutConnection.connected()) {
        timeoutConnection.disconnect();
    }

    onPop();
}

void Screen::setTimeout(std::chrono::milliseconds duration) {
    timeoutConnection = Glib::signal_timeout().connect(
        [this]() -> bool {
            if (mainWindow && mainWindow->isCurrentScreen(*this)) {
                mainWindow->popScreen();
            }
            return false;
        },
        duration.count());
}
