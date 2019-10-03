#pragma once

#include <memory>

#include "access_control/access_control.h"
#include "ui/screen.h"

class KeyScreen : public Screen {
public:
    KeyScreen(AccessControl* accessControl);
    ~KeyScreen() override;

    Gtk::Widget& widget() override;
    std::vector<ScreenButton> buttons() override;
    bool handleScannedKey(std::string const& key) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
