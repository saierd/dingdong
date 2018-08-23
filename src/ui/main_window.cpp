#include "main_window.h"

#include "gtk_helpers.h"

std::string const windowBackgroundColor = "#FFF";
int const mainWindowPadding = 30;

int const applicationFontSize = 40;

MainWindow::MainWindow() {
    set_border_width(mainWindowPadding);
    fullscreen();

    setBackgroundColor(*this, windowBackgroundColor);
    setFont(*this, applicationFontSize, true);

    vbox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    vbox.set_spacing(2 * mainWindowPadding);
    vbox.pack_end(footerBox, Gtk::PACK_SHRINK);

    logo.set_from_resource("/logo.png");
    footerBox.pack_end(logo, Gtk::PACK_SHRINK);

    add(vbox);
    show_all();

#ifdef RASPBERRY_PI
    // Hide the cursor when we have a touchscreen.
    get_window()->set_cursor(Gdk::Cursor::create(Gdk::BLANK_CURSOR));
#endif
}

void MainWindow::showScreen(Screen& screen) {
    if (currentScreen != nullptr) {
        vbox.remove(currentScreen->widget());
    }
    vbox.pack_start(screen.widget());
    currentScreen = &screen;
}

bool MainWindow::isCurrentScreen(Screen const& screen) const {
    return &screen == currentScreen;
}
