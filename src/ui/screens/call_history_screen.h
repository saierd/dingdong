#pragma once

#include "ui/screen.h"

class CallHistory;

class CallHistoryScreen : public BaseScreen {
public:
    CallHistoryScreen();
    ~CallHistoryScreen() override;

    Gtk::Widget& widget() override;
    std::vector<ScreenButton> buttons() override;

    void updateHistory(CallHistory const& history);

    sigc::signal<void> onClear;

private:
    void onShow() override;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
