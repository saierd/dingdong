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

void MainScreen::updateInstances(std::vector<Instance> _instances) {
    instances = std::move(_instances);

    instanceButtons.clear();

    int row = 0;
    int column = 0;
    for (auto const& instance : instances) {
        instanceButtons.emplace_back(instance.name());
        styleButton(instanceButtons.back(), largePadding);
        setFont(instanceButtons.back(), largeFontSize, true);

        instanceButtons.back().signal_clicked().connect([this, &instance]() { onCall(instance); });
        buttonGrid.attach(instanceButtons.back(), column, row, 1, 1);

        row++;
        if (row >= instancesPerColumn) {
            column++;
            row = 0;
        }
    }

    buttonGrid.show_all();
}
