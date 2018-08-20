#include "main_screen.h"

MainScreen::MainScreen() {
    buttonBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    buttonBox.set_spacing(10);
}

Gtk::Widget& MainScreen::widget() {
    return buttonBox;
}

void MainScreen::updateInstances(std::vector<Instance> _instances) {
    instances = std::move(_instances);

    buttons.clear();
    for (auto const& instance : instances) {
        buttons.emplace_back(instance.name());
        buttons.back().signal_clicked().connect([this, &instance]() {
            onCall(instance);
        });
        buttonBox.pack_start(buttons.back());
    }
    buttonBox.show_all();
}
