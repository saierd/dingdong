#include "key_input_screen.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>

#include "system/beep.h"

#include "ui/constants.h"
#include "ui/gtk_helpers.h"
#include "ui/main_window.h"

int const buttonSize = 80;

class KeyInputScreen::Impl {
public:
    Impl(bool needConfirmation) : _needConfirmation(needConfirmation) {
        grid.set_row_spacing(defaultSpacing);
        grid.set_column_spacing(defaultSpacing);

        for (int i = 0; i <= 9; i++) {
            buttons.emplace_back(std::to_string(i));
            buttons.back().set_size_request(buttonSize, buttonSize);
            styleButton(buttons.back());
            setFont(buttons.back(), largeFontSize);
            buttons.back().signal_clicked().connect([this, i]() { addNumber(i); });

            int row = (9 - i) / 3;
            int column = 1 + (i - 1) % 3;
            if (i == 0) {
                row = 2;
                column = 0;
            }
            grid.attach(buttons.back(), column, row, 1, 1);
        }

        clearButton.set_label("C");
        clearButton.set_size_request(buttonSize, buttonSize);
        styleButton(clearButton, colorRed);
        setFont(clearButton, largeFontSize);
        clearButton.signal_clicked().connect([this]() {
            beep();
            clear();
        });
        grid.attach(clearButton, 4, 2, 1, 1);

        box.set_center_widget(grid);
        box.show_all();
    }

    void addNumber(int number) {
        beep();

        key += std::to_string(number);
        if (onInput) {
            onInput();
        }
    }

    void clear() {
        key = "";
    }

public:
    std::string key;

    bool _needConfirmation;
    std::function<void()> onInput;

    Gtk::Box box;

    Gtk::Grid grid;
    Gtk::Button clearButton;
    std::vector<Gtk::Button> buttons;
};

KeyInputScreen::KeyInputScreen(bool needConfirmation) {
    impl = std::make_unique<Impl>(needConfirmation);

    impl->onInput = [this]() {
        if (impl->_needConfirmation || impl->key.empty()) return;
        onKeyInput.emit(impl->key);
    };
}

KeyInputScreen::~KeyInputScreen() = default;

Gtk::Widget& KeyInputScreen::widget() {
    return impl->box;
}

std::vector<ScreenButton> KeyInputScreen::buttons() {
    ScreenButton backButton([this]() { mainWindow->popScreen(); });
    backButton.icon = "/back.svg";

    if (!impl->_needConfirmation) {
        return { backButton };
    }

    ScreenButton confirmButton([this]() {
        mainWindow->popScreen();
        onKeyInput.emit(impl->key);
    });
    confirmButton.icon = "/check.svg";
    confirmButton.color = colorGreen;

    return { backButton, confirmButton };
}

void KeyInputScreen::onShow() {
    impl->clear();
}
