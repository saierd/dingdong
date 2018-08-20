#pragma once

#include <gtkmm/widget.h>
#include <gtkmm/cssprovider.h>

void applyCss(Gtk::Widget& widget, std::string const& css) {
    auto cssProvider = Gtk::CssProvider::create();
    cssProvider->load_from_data(css);

    widget.get_style_context()->add_provider(cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
