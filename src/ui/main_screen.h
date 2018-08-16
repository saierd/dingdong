#pragma once

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>

#include "screen.h"
#include "instance.h"

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
