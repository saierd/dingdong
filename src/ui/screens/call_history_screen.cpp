#include "call_history_screen.h"

#include <vector>

#include <spdlog/fmt/fmt.h>

#include <gdkmm/pixbufloader.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "call_history.h"
#include "ui/gtk_helpers.h"
#include "ui/main_window.h"

class CallHistoryScreen::Impl {
public:
    Impl() {
        setFont(label, largeFontSize, true);

        loadButtonIcon(backButtonIcon, "/back.svg");
        backButton.set_image(backButtonIcon);
        styleButton(backButton);
        backButton.set_size_request(buttonMinimumWidth);

        backButton.signal_clicked().connect([this]() {
            currentEntryIndex--;
            updateVisibleEntry();
        });

        loadButtonIcon(forwardButtonIcon, "/forward.svg");
        forwardButton.set_image(forwardButtonIcon);
        styleButton(forwardButton);
        forwardButton.set_size_request(buttonMinimumWidth);

        forwardButton.signal_clicked().connect([this]() {
            currentEntryIndex++;
            updateVisibleEntry();
        });

        hbox.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
        hbox.set_spacing(defaultSpacing);
        hbox.pack_start(backButton, false, false);
        hbox.pack_start(image);
        hbox.pack_start(forwardButton, false, false);

        box.set_orientation(Gtk::ORIENTATION_VERTICAL);
        box.set_spacing(defaultSpacing);
        box.pack_start(label);
        box.pack_start(hbox);
        box.show_all();
    }

    void updateEntries(std::vector<std::shared_ptr<CallHistory::Entry>> newEntries) {
        entries = std::move(newEntries);

        if (currentEntryIndex >= entries.size()) {
            currentEntryIndex = 0;
        }
        updateVisibleEntry();
    }

    void updateVisibleEntry() {
        backButton.set_sensitive(currentEntryIndex > 0);
        forwardButton.set_sensitive(!entries.empty() && currentEntryIndex < (entries.size() - 1));

        if (entries.empty()) return;

        label.set_text(fmt::format("{} / {}", currentEntryIndex + 1, entries.size()));

        auto const& entry = *entries[currentEntryIndex];

        auto loader = Gdk::PixbufLoader::create();
        loader->write(reinterpret_cast<guint8 const*>(entry.image.c_str()), entry.image.size());
        loader->close();

        auto pixbuf = loader->get_pixbuf();
        image.set(pixbuf);
    }

    std::vector<std::shared_ptr<CallHistory::Entry>> entries;
    size_t currentEntryIndex = 0;

    Gtk::Box box;
    Gtk::Box hbox;

    Gtk::Label label;
    Gtk::Image image;

    Gtk::Button backButton;
    Gtk::Image backButtonIcon;
    Gtk::Button forwardButton;
    Gtk::Image forwardButtonIcon;
};

CallHistoryScreen::CallHistoryScreen() {
    impl = std::make_unique<Impl>();
}

CallHistoryScreen::~CallHistoryScreen() = default;

Gtk::Widget& CallHistoryScreen::widget() {
    return impl->box;
}

std::vector<ScreenButton> CallHistoryScreen::buttons() {
    ScreenButton backButton([this]() { mainWindow->popScreen(); });
    backButton.icon = "/back.svg";

    ScreenButton clearButton([this]() {
        onClear.emit();
        mainWindow->popScreen();
    });
    clearButton.icon = "/delete.svg";
    clearButton.color = colorRed;

    return { backButton, clearButton };
}

void CallHistoryScreen::updateHistory(CallHistory const& history) {
    impl->updateEntries(history.getMissedEntries());
}

void CallHistoryScreen::onShow() {
    impl->currentEntryIndex = 0;
    impl->updateVisibleEntry();
}
