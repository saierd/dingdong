#pragma once

#include <gtkmm/widget.h>
#include <gtkmm/cssprovider.h>

inline void applyCss(Gtk::Widget& widget, std::string const& css) {
    auto cssProvider = Gtk::CssProvider::create();
    cssProvider->load_from_data(css);

    widget.get_style_context()->add_provider(cssProvider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

inline void setBackgroundColor(Gtk::Widget& widget, std::string const& color) {
    std::string css = "* { background-color: " + color + "; }";
    applyCss(widget, css);
}

inline void setFont(Gtk::Widget& widget, int fontSize, bool bold = false) {
    std::string css = "* {" \
        "font-size: " + std::to_string(fontSize) + "px;" \
        "font-weight: " + (bold ? "bold" : "normal") + ";" \
    "}";
    applyCss(widget, css);
}

inline void styleButton(Gtk::Widget& widget, std::string const& color, std::string const& fontColor = "#FFF", int padding = 0) {
    std::string css = "* {" \
        "background: " + color + ";" \
        "border: none;" \
        "border-radius: 10px;" \
        "color: " + fontColor + ";" \
        "padding-top: " + std::to_string(padding) + "px;" \
        "padding-bottom: " + std::to_string(padding) + "px;" \
    "}";
    applyCss(widget, css);
}
