#include "main_window.h"

#include "constants.h"
#include "gtk_helpers.h"

std::string const windowBackgroundColor = colorWhite;
int const mainWindowPadding = 30;

int const buttonMinimumWidth = 100;

MainWindow::MainWindow(Screen& baseScreen) {
    set_border_width(mainWindowPadding);
    fullscreen();

    setBackgroundColor(*this, windowBackgroundColor);
    setFont(*this, applicationFontSize);

    vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    vbox.set_spacing(2 * mainWindowPadding);
    vbox.pack_end(footerBox, Gtk::PACK_SHRINK);

    logo.set_from_resource("/logo.png");
    footerBox.set_spacing(defaultSpacing);
    footerBox.pack_end(logo, Gtk::PACK_SHRINK);

    add(vbox);
    show_all();

#ifdef RASPBERRY_PI
    // Hide the cursor when we have a touchscreen.
    get_window()->set_cursor(Gdk::Cursor::create(Gdk::BLANK_CURSOR));
#endif

    pushScreen(baseScreen);
}

void MainWindow::addPermanentButton(ScreenButton button) {
    permanentButtons.push_back(button);
    updateButtons();
}

void MainWindow::clearPermanentButtons() {
    permanentButtons.clear();
    updateButtons();
}

void MainWindow::pushScreen(Screen& screen) {
    screenStack.push_back(&screen);
    showScreen(screenStack.back());
}

void MainWindow::popScreen() {
    if (screenStack.size() <= 1) return;

    screenStack.pop_back();
    showScreen(screenStack.back());
}

bool MainWindow::isCurrentScreen(Screen const& screen) const {
    return &screen == currentScreen;
}

bool MainWindow::handleScannedKey(std::string const& key) const {
    if (!currentScreen) return false;
    return currentScreen->handleScannedKey(key);
}

void MainWindow::showScreen(Screen* screen) {
    if (currentScreen == screen) return;

    if (currentScreen != nullptr) {
        vbox.remove(currentScreen->widget());
        currentScreenConnection.disconnect();
    }

    vbox.pack_start(screen->widget());
    currentScreen = screen;
    screen->mainWindow = this;
    currentScreenConnection = currentScreen->onButtonsChanged.connect([this]() { updateButtons(); });
    updateButtons();
}

void MainWindow::updateButtons() {
    buttons.clear();
    buttonIcons.clear();

    auto addButton = [this](ScreenButton const& button) {
        buttons.emplace_back(button.caption);

        if (!button.icon.empty()) {
            buttonIcons.resize(buttonIcons.size() + 1);
            loadButtonIcon(buttonIcons.back(), button.icon);
            buttons.back().set_image(buttonIcons.back());
            buttons.back().set_always_show_image(true);
        }

        styleButton(buttons.back(), button.color, button.fontColor);
        buttons.back().signal_clicked().connect([callback = button.callback]() { callback(); });
        buttons.back().set_size_request(buttonMinimumWidth);
        footerBox.pack_start(buttons.back(), Gtk::PACK_SHRINK);
    };

    if (currentScreen) {
        for (auto const& button : currentScreen->buttons()) {
            addButton(button);
        }
    }
    for (auto const& button : permanentButtons) {
        addButton(button);
    }
    footerBox.show_all();
}
