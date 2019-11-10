#pragma once

#include <memory>
#include <vector>

#include <glibmm.h>

#include "ui/screen.h"

class KeyInputScreen : public Screen {
public:
    KeyInputScreen(bool needConfirmation);
    ~KeyInputScreen() override;

    Gtk::Widget& widget() override;
    std::vector<ScreenButton> buttons() override;

private:
    void onShow() override;

public:
    sigc::signal<void, std::string> onKeyInput;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
