#include "gstreamer.h"

#include "gstreamer_helpers.h"

void initializeGStreamer(int argc, char** argv) {
    gst_init(&argc, &argv);
}

class GStreamerMainLoop::Impl {
public:
    GMainLoop* loop = nullptr;
};

GStreamerMainLoop::GStreamerMainLoop() {
    impl = std::make_unique<Impl>();
    impl->loop = g_main_loop_new(nullptr, false);
}

GStreamerMainLoop::~GStreamerMainLoop() {
    if (impl->loop) g_main_loop_unref(impl->loop);
}

void GStreamerMainLoop::run() {
    g_main_loop_run(impl->loop);
}
