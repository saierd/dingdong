#pragma once

#include <memory>
#include <vector>

#include <gtkmm/button.h>
#include <gtkmm/grid.h>

#include "access_control/action.h"
#include "ui/screen.h"

class ActionScreen : public Screen {
public:
    ActionScreen();

    Gtk::Widget& widget() override;
    std::vector<ScreenButton> buttons() override;

    void onShow() override;
    void onPop() override;

    void updateActions(std::vector<std::shared_ptr<Action>> const& actions);

private:
    Gtk::Grid buttonGrid;
    std::vector<Gtk::Button> actionButtons;

    sigc::connection timeoutConnection;
};
