#include "action_screen.h"

#include <chrono>

#include <glibmm.h>

#include "ui/constants.h"
#include "ui/gtk_helpers.h"
#include "ui/main_window.h"

int const actionsPerColumn = 3;

std::chrono::seconds screenTimeout(20);

// When triggering an action that allows triggering additional actions, this is the time the action screen stays visible
// before it will get popped.
std::chrono::seconds screenHideTimeout(2);

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
        actionButtons.emplace_back(std::make_shared<Gtk::Button>(action->caption()));
        std::shared_ptr<Gtk::Button> button = actionButtons.back();

        styleButton(*button, largePadding);
        setFont(*button, mediumFontSize, true);

        button->signal_clicked().connect([this, action, buttonReference = std::weak_ptr<Gtk::Button>(button)]() {
            if (action->allowAdditionalAction()) {
                // Prevent triggering the same action twice.
                if (auto actionButton = buttonReference.lock(); actionButton) {
                    actionButton->set_sensitive(false);
                    styleButton(*actionButton, colorLightGrey, largePadding);
                }

                // Pop the action screen after a delay to allow the user to trigger another action.
                if (!didTriggerScreenPop) {
                    Glib::signal_timeout().connect_once(
                        [this]() {
                            // Since this happens with a delay, we need to check whether the current screen is still the
                            // action screen that we wanted to pop. The user could have triggered another action, which
                            // popped the action screen immediately in the mean time.
                            if (mainWindow && mainWindow->isCurrentScreen(*this)) {
                                mainWindow->popScreen();
                            }
                        },
                        std::chrono::duration_cast<std::chrono::milliseconds>(screenHideTimeout).count());
                    didTriggerScreenPop = true;
                }
            } else {
                // Pop the action screen immediately.
                mainWindow->popScreen();
            }

            action->trigger();
        });
        buttonGrid.attach(*button, column, row, 1, 1);

        row++;
        if (row >= actionsPerColumn) {
            column++;
            row = 0;
        }
    }

    buttonGrid.show_all();
}

void ActionScreen::onShow() {
    didTriggerScreenPop = false;
    setTimeout(screenTimeout);
}
