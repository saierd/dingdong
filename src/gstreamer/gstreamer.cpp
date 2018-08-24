#include "gstreamer.h"

#include "gstreamer_helpers.h"

void initializeGStreamer(int argc, char** argv) {
    gst_init(&argc, &argv);
}
