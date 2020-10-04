#pragma once

#include <gtkmm/aspectframe.h>
#include <gtkmm/image.h>

class ScalingImageContent : public Gtk::Image {
public:
    void set(Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
        originalImage = pixbuf;
        updateScaledImage();
    }

protected:
    void on_size_allocate(Gtk::Allocation& allocation) override {
        currentSize = allocation;
        updateScaledImage();

        Gtk::Image::on_size_allocate(allocation);
    }

private:
    void updateScaledImage() {
        if (currentSize.get_width() == 0 || currentSize.get_height() == 0) {
            // No size allocated for the widget yet. Ignore this signal, we will adjust the image as soon as we get some
            // space allocated.
            return;
        }

        if (originalImage == currentScaledImageFromOriginalImage &&
            currentSize.get_width() == get_pixbuf()->get_width() &&
            currentSize.get_height() == get_pixbuf()->get_height()) {
            // Nothing to do, image has not changed and already has the correct size. This catches spurious invocations
            // of on_size_allocate, which gets called again when we set the scaled image below. It also gets called when
            // the widget gets displayed again with the same size as before.
            return;
        }

        Glib::RefPtr<Gdk::Pixbuf> scaledImage =
            originalImage->scale_simple(currentSize.get_width(), currentSize.get_height(), Gdk::INTERP_BILINEAR);
        Gtk::Image::set(scaledImage);
        currentScaledImageFromOriginalImage = originalImage;
    }

    Gtk::Allocation currentSize;
    Glib::RefPtr<Gdk::Pixbuf> originalImage;
    Glib::RefPtr<Gdk::Pixbuf> currentScaledImageFromOriginalImage;
};

class ScalingImage : public Gtk::AspectFrame {
public:
    ScalingImage() {
        set_shadow_type(Gtk::SHADOW_NONE);
        add(image);
    }

    void set(Glib::RefPtr<Gdk::Pixbuf> pixbuf) {
        // Adjust aspect ratio to the ratio of the image.
        Gtk::AspectFrame::set(Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER,
                              static_cast<float>(pixbuf->get_width()) / pixbuf->get_height());

        image.set(pixbuf);
    }

private:
    ScalingImageContent image;
};
