#include "action_screen.h"

#include "ui/constants.h"
#include "ui/gtk_helpers.h"
#include "ui/main_window.h"

ActionScreen::ActionScreen() {
    buttonBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    buttonBox.set_spacing(defaultSpacing);
}

Gtk::Widget& ActionScreen::widget() {
    return buttonBox;
}

std::vector<ScreenButton> ActionScreen::buttons() {
    ScreenButton backButton([this]() { mainWindow->popScreen(); });
    backButton.icon = "/back.svg";

    return { backButton };
}

void ActionScreen::updateActions(std::vector<std::shared_ptr<Action>> const& actions) {
    actionButtons.clear();
    for (auto const& action : actions) {
        actionButtons.emplace_back(action->caption());
        styleButton(actionButtons.back(), largePadding);
        setFont(actionButtons.back(), largeFontSize, true);

        actionButtons.back().signal_clicked().connect([action, this]() {
            // Pop the action screen.
            mainWindow->popScreen();

            action->trigger();
        });
        buttonBox.pack_start(actionButtons.back(), Gtk::PACK_SHRINK);
    }
    buttonBox.show_all();
}
