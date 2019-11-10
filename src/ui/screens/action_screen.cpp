#include "action_screen.h"

#include <chrono>

#include "ui/constants.h"
#include "ui/gtk_helpers.h"
#include "ui/main_window.h"

int const actionsPerColumn = 3;

std::chrono::seconds actionScreenTimeout(20);

ActionScreen::ActionScreen() {
    buttonGrid.set_column_homogeneous(true);
    buttonGrid.set_row_spacing(defaultSpacing);
    buttonGrid.set_column_spacing(defaultSpacing);
}

Gtk::Widget& ActionScreen::widget() {
    return buttonGrid;
}

std::vector<ScreenButton> ActionScreen::buttons() {
    ScreenButton backButton([this]() { mainWindow->popScreen(); });
    backButton.icon = "/back.svg";

    return { backButton };
}

void ActionScreen::updateActions(std::vector<std::shared_ptr<Action>> const& actions) {
    actionButtons.clear();

    int row = 0;
    int column = 0;
    for (auto const& action : actions) {
        actionButtons.emplace_back(action->caption());
        styleButton(actionButtons.back(), largePadding);
        setFont(actionButtons.back(), mediumFontSize, true);

        actionButtons.back().signal_clicked().connect([action, this]() {
            // Pop the action screen.
            mainWindow->popScreen();

            action->trigger();
        });
        buttonGrid.attach(actionButtons.back(), column, row, 1, 1);

        row++;
        if (row >= actionsPerColumn) {
            column++;
            row = 0;
        }
    }

    buttonGrid.show_all();
}

void ActionScreen::onShow() {
    setTimeout(actionScreenTimeout);
}
