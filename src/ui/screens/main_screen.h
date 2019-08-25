#pragma once

#include <vector>

#include <gtkmm/button.h>
#include <gtkmm/grid.h>

#include "instance.h"
#include "ui/screen.h"

class MainScreen : public Screen {
public:
    MainScreen();

    Gtk::Widget& widget() override;

    void updateInstances(std::vector<Instance> instances);
    sigc::signal<void, Instance const&> onCall;

private:
    std::vector<Instance> instances;

    Gtk::Grid buttonGrid;
    std::vector<Gtk::Button> instanceButtons;
};
