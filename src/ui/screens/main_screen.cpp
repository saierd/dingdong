#include "main_screen.h"

#include "ui/constants.h"
#include "ui/gtk_helpers.h"

MainScreen::MainScreen() {
    buttonBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    buttonBox.set_spacing(defaultSpacing);
}

Gtk::Widget& MainScreen::widget() {
    return buttonBox;
}

void MainScreen::updateInstances(std::vector<Instance> _instances) {
    instances = std::move(_instances);

    instanceButtons.clear();
    for (auto const& instance : instances) {
        instanceButtons.emplace_back(instance.name());
        styleButton(instanceButtons.back(), largePadding);
        setFont(instanceButtons.back(), largeFontSize, true);

        instanceButtons.back().signal_clicked().connect([this, &instance]() { onCall(instance); });
        buttonBox.pack_start(instanceButtons.back(), Gtk::PACK_SHRINK);
    }
    buttonBox.show_all();
}
