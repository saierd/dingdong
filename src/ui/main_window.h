#pragma once

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>

#include "instance.h"

class MainWindow : public Gtk::Window {
public:
    MainWindow();

    void updateInstances(std::vector<Instance> instances);
    sigc::signal<void, Instance const&> onCall;

private:
    std::vector<Instance> instances;

    Gtk::Box buttonBox;
    std::vector<Gtk::Button> buttons;
};
