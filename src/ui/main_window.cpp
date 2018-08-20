#include "main_window.h"

MainWindow::MainWindow() {
    set_border_width(20);
}

void MainWindow::showScreen(Screen& screen) {
    remove();
    add(screen.widget());
    currentScreen = &screen;
}

bool MainWindow::isCurrentScreen(Screen const& screen) const {
    return &screen == currentScreen;
}
