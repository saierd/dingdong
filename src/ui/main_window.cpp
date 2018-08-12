#include "main_window.h"

MainWindow::MainWindow() {
    set_border_width(10);

    buttonBox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    add(buttonBox);
}

void MainWindow::updateInstances(std::vector<Instance> _instances) {
    instances = std::move(_instances);

    buttons.clear();
    for (auto const& instance : instances) {
        buttons.emplace_back(instance.name());
        buttons.back().signal_clicked().connect([this, &instance]() {
            onCall(instance);
        });
        buttonBox.pack_end(buttons.back());
    }
    show_all_children();
}
