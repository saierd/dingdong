#pragma once

#include <functional>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/window.h>

#include "screen.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow(Screen& baseScreen);

    void addPermanentButton(ScreenButton button, std::function<bool()> show);
    void addPermanentButton(ScreenButton button);
    void clearPermanentButtons();

    void pushScreen(Screen& screen);
    void popScreen();
    bool isCurrentScreen(Screen const& screen) const;

    // Notify the current screen about a scanned key. Returns true if the screen handled the key.
    bool handleScannedKey(std::string const& key) const;

    sigc::signal<void> onScreenChanged;

private:
    void showScreen(Screen* screen);
    void updateButtons();

private:
    Screen* currentScreen = nullptr;
    std::vector<Screen*> screenStack;

    // Connection to signals of the current screen.
    sigc::connection currentScreenConnection;

    struct PermanentButton {
        ScreenButton button;
        std::function<bool()> show;
    };

    std::vector<PermanentButton> permanentButtons;

    Gtk::Box vbox;
    Gtk::Box footerBox;
    std::vector<Gtk::Button> buttons;
    std::vector<Gtk::Image> buttonIcons;
    Gtk::Image blahIcon;
    Gtk::Image logo;
};
