#pragma once

#include <chrono>
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

    void notifyShown();
    void notifyPopped();

    virtual bool handleScannedKey(std::string const& /*unused*/) {
        return false;
    }

    sigc::signal<void> onButtonsChanged;

private:
    virtual void onShow(){
        // This method gets called after the screen got shown in the main window.
    };

    virtual void onPop() {
        // This method gets called once the screen got popped from the main window.
    }

protected:
    // Set the timeout for the current screen. When the timeout triggers and the screen is still visible, it will get
    // popped automatically.
    void setTimeout(std::chrono::milliseconds duration);

protected:
    friend class MainWindow;
    MainWindow* mainWindow;

private:
    sigc::connection timeoutConnection;
};
