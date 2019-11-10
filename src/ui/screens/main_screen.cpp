#include "main_screen.h"

#include "ui/constants.h"
#include "ui/gtk_helpers.h"

int const instancesPerColumn = 3;

MainScreen::MainScreen() {
    buttonGrid.set_column_homogeneous(true);
    buttonGrid.set_row_spacing(defaultSpacing);
    buttonGrid.set_column_spacing(defaultSpacing);
}

Gtk::Widget& MainScreen::widget() {
    return buttonGrid;
}

void MainScreen::updateInstances(std::vector<Instance> const& instances, std::vector<CallInfo> const& calls) {
    instanceButtons.clear();
    instanceButtonBoxes.clear();
    instanceButtonIcons.clear();
    instanceButtonLabels.clear();

    int row = 0;
    int column = 0;
    for (auto const& instance : instances) {
        std::optional<CallInfo> callToInstance;
        for (auto const& call : calls) {
            if (call.targetId == instance.id()) {
                callToInstance = call;
                break;
            }
        }

        std::string buttonColor = colorDarkGrey;
        std::string buttonIcon = "/bell.svg";
        if (callToInstance) {
            if (callToInstance->isRunning) {
                buttonColor = colorGreen;
                buttonIcon = "/speaker.svg";
            } else {
                buttonColor = colorYellow;
                buttonIcon = "/bell_ring.svg";
            }
        }

        instanceButtons.emplace_back();
        styleButton(instanceButtons.back(), buttonColor, largePadding);
        setFont(instanceButtons.back(), largeFontSize, true);

        instanceButtonBoxes.emplace_back();
        Gtk::Box& hbox = instanceButtonBoxes.back();
        hbox.set_spacing(defaultSpacing);

        if (!buttonIcon.empty()) {
            instanceButtonIcons.resize(instanceButtonIcons.size() + 1);
            loadButtonIcon(instanceButtonIcons.back(), buttonIcon);
            instanceButtonIcons.back().set_margin_left(defaultSpacing);
            hbox.pack_start(instanceButtonIcons.back(), false, false);
        }

        instanceButtonLabels.emplace_back(instance.name());
        hbox.pack_start(instanceButtonLabels.back(), true, true);

        instanceButtons.back().add(hbox);

        instanceButtons.back().signal_clicked().connect([this, instance]() { onCall(instance); });
        buttonGrid.attach(instanceButtons.back(), column, row, 1, 1);

        row++;
        if (row >= instancesPerColumn) {
            column++;
            row = 0;
        }
    }

    buttonGrid.show_all();
}
