#pragma once

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>

#include "instance.h"
#include "screen.h"

class MainScreen : public Screen {
public:
    MainScreen();

    Gtk::Widget& widget() override;

    void updateInstances(std::vector<Instance> instances);
    sigc::signal<void, Instance const&> onCall;

private:
    std::vector<Instance> instances;

    Gtk::Box buttonBox;
    std::vector<Gtk::Button> buttons;
};
