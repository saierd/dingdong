#include "main_screen.h"

#include "gtk_helpers.h"

std::string const instanceButtonColor = "#555";
std::string const instanceButtonFontColor = "#FFF";
int const instanceButtonPadding = 25;
int const instanceButtonSpacing = 20;

MainScreen::MainScreen() {
    buttonBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    buttonBox.set_spacing(instanceButtonSpacing);
}

Gtk::Widget& MainScreen::widget() {
    return buttonBox;
}

void MainScreen::updateInstances(std::vector<Instance> _instances) {
    instances = std::move(_instances);

    buttons.clear();
    for (auto const& instance : instances) {
        buttons.emplace_back(instance.name());
        styleButton(buttons.back(), instanceButtonColor, instanceButtonFontColor, instanceButtonPadding);

        buttons.back().signal_clicked().connect([this, &instance]() {
            onCall(instance);
        });
        buttonBox.pack_start(buttons.back(), Gtk::PACK_SHRINK);
    }
    buttonBox.show_all();
}
