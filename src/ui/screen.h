#pragma once

#include <functional>
#include <string>
#include <vector>

#include <gtkmm/widget.h>

#include "constants.h"

class MainWindow;

struct ScreenButton {
    std::string caption;
    std::string icon;
    std::string color = colorDarkGrey;
    std::string fontColor = colorWhite;
    std::function<void()> callback;

    ScreenButton(std::function<void()> _callback) : callback(std::move(_callback)) {}
    ScreenButton(std::string _caption, std::function<void()> _callback)
        : caption(std::move(_caption)), callback(std::move(_callback)) {}
};

class Screen {
public:
    virtual ~Screen() = default;

    virtual Gtk::Widget& widget() = 0;

    virtual std::vector<ScreenButton> buttons() {
        return {};
    }

    virtual void onShow(){
        // This method gets called after the screen got shown in the main window.
    };

    virtual void onPop() {
        // This method gets called once the screen got popped from the main window.
    }

    virtual bool handleScannedKey(std::string const& /*unused*/) {
        return false;
    }

    sigc::signal<void> onButtonsChanged;

protected:
    friend class MainWindow;
    MainWindow* mainWindow;
};
