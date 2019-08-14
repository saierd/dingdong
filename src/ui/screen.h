#pragma once

#include <gtkmm/widget.h>

class Screen {
public:
    virtual ~Screen() = default;

    virtual Gtk::Widget& widget() = 0;
};
