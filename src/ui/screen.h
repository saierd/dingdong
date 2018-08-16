#pragma once

#include <gtkmm/widget.h>

class Screen {
public:
    virtual ~Screen() {}

    virtual Gtk::Widget& widget() = 0;
};
