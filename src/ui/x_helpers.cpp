#include "x_helpers.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

void hideCursorForApplication() {
    Display* conn = XOpenDisplay(nullptr);
    XFixesHideCursor(conn, DefaultRootWindow(conn));
    XFlush(conn);
}
