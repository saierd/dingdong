#include "main_window.h"

#include "constants.h"
#include "gtk_helpers.h"

std::string const windowBackgroundColor = colorWhite;
int const mainWindowPadding = largePadding;

int const buttonMinimumWidth = 100;

MainWindow::MainWindow(BaseScreen& baseScreen) {
    set_border_width(mainWindowPadding);
    fullscreen();

    setBackgroundColor(*this, windowBackgroundColor);
    setFont(*this, applicationFontSize);

    vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    vbox.set_spacing(mainWindowPadding);
    vbox.pack_end(footerBox, Gtk::PACK_SHRINK);

    logo.set_from_resource("/logo.png");
    footerBox.set_spacing(defaultSpacing);
    footerBox.pack_end(logo, Gtk::PACK_SHRINK);

    add(vbox);
    show_all();

#ifdef RASPBERRY_PI
    // Keep the window on top.
    get_window()->set_keep_above();
#endif

    pushScreen(baseScreen);
}

void MainWindow::addPermanentButton(ScreenButton button, std::function<bool()> show) {
    permanentButtons.push_back({ std::move(button), std::move(show) });
    updateButtons();
}

void MainWindow::addPermanentButton(ScreenButton button) {
    addPermanentButton(std::move(button), []() { return true; });
}

void MainWindow::clearPermanentButtons() {
    permanentButtons.clear();
    updateButtons();
}

void MainWindow::pushScreen(BaseScreen& screen) {
    screenStack.push_back(&screen);
    showScreen(screenStack.back());
    onScreenChanged();
}

void MainWindow::popScreen() {
    if (screenStack.size() <= 1) return;

    screenStack.back()->notifyPopped();
    screenStack.pop_back();

    showScreen(screenStack.back());
    onScreenChanged();
}

bool MainWindow::isCurrentScreen(BaseScreen const& screen) const {
    return &screen == currentScreen;
}

bool MainWindow::handleScannedKey(std::string const& key) const {
    if (!currentScreen) return false;
    return currentScreen->handleScannedKey(key);
}

void MainWindow::showScreen(BaseScreen* screen) {
    if (currentScreen == screen) return;

    if (currentScreen != nullptr) {
        vbox.remove(currentScreen->widget());
        currentScreenConnection.disconnect();
    }

    vbox.pack_start(screen->widget());
    currentScreen = screen;
    screen->mainWindow = this;
    screen->notifyShown();

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
        if (button.show()) {
            addButton(button.button);
        }
    }
    footerBox.show_all();
}
