#!/bin/bash

gtk_css_file="$HOME/.config/gtk-3.0/gtk.css"

hardware_name=$(uname -m)
if [ "${hardware_name}" == "armv7l" ]; then
    # Disable some Raspbian GTK settings that prevent overriding the font size from applications.
    sed -i '/\/\*/! s/^.*font.*/\/\*\0\*\//' "${gtk_css_file}"
fi

