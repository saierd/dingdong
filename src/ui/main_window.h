#pragma once

#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/window.h>

#include "screen.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow();

    void showScreen(Screen& screen);
    bool isCurrentScreen(Screen const& screen) const;

private:
    Screen* currentScreen = nullptr;

    Gtk::Box vbox;
    Gtk::Box footerBox;
    Gtk::Image logo;
};
