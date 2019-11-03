#pragma once

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "call_protocol.h"
#include "instance.h"
#include "ui/screen.h"

class MainScreen : public Screen {
public:
    MainScreen();

    Gtk::Widget& widget() override;

    void updateInstances(std::vector<Instance> const& instances, std::vector<CallInfo> const& calls);
    sigc::signal<void, Instance const&> onCall;

private:
    Gtk::Grid buttonGrid;
    std::vector<Gtk::Button> instanceButtons;
    std::vector<Gtk::Box> instanceButtonBoxes;
    std::vector<Gtk::Image> instanceButtonIcons;
    std::vector<Gtk::Label> instanceButtonLabels;
};
