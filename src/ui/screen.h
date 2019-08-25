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
        // This method gets called once when the screen gets changed.
    };

    virtual bool handleScannedKey(std::string const&) {
        return false;
    }

    sigc::signal<void> onButtonsChanged;

protected:
    friend class MainWindow;
    MainWindow* mainWindow;
};
