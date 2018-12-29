#pragma once

#include <gdkmm/pixbuf.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/image.h>
#include <gtkmm/widget.h>

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
    // clang-format off
    std::string css =
        "* {"
            "font-size: " + std::to_string(fontSize) + "px;"
            "font-weight: " + (bold ? "bold" : "normal") + ";"
        "}";
    // clang-format on
    applyCss(widget, css);
}

inline void styleButton(Gtk::Widget& widget, std::string const& color, std::string const& fontColor = "#FFF",
                        int padding = 0) {
    // clang-format off
    std::string css =
        "* {"
            "background: " + color + ";"
            "border: none;"
            "border-radius: 10px;"
            "color: " + fontColor + ";"
            "padding-top: " + std::to_string(padding) + "px;"
            "padding-bottom: " + std::to_string(padding) + "px;"
        "}";
    // clang-format on
    applyCss(widget, css);
}

inline void loadImageWithSize(Gtk::Image& image, std::string const& resourcePath, int width, int height = 0,
                              bool forceWhite = false) {
    auto pixbuf = Gdk::Pixbuf::create_from_resource(resourcePath, width, height ? height : width);

    if (forceWhite) {
        auto data = pixbuf->get_pixels();
        for (int i = 0; i < pixbuf->get_width() * pixbuf->get_height(); i++) {
            data[0] = 255;
            data[1] = 255;
            data[2] = 255;
            // Don't touch transparency.
            data += 4;
        }
    }

    image.set(pixbuf);
}
