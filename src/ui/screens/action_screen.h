#pragma once

#include <memory>
#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/button.h>

#include "access_control/action.h"
#include "ui/screen.h"

class ActionScreen : public Screen {
public:
    ActionScreen();

    Gtk::Widget& widget() override;
    std::vector<ScreenButton> buttons() override;

    void updateActions(std::vector<std::shared_ptr<Action>> const& actions);

private:
    Gtk::Box buttonBox;
    std::vector<Gtk::Button> actionButtons;
};
