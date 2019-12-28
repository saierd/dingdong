#include "gstreamer.h"

#include "gstreamer_helpers.h"
#include "util/logging.h"

void gstreamerLogFunction(GstDebugCategory* /*category*/, GstDebugLevel level, const gchar* /*file*/,
                          const gchar* /*function*/, gint /*line*/, GObject* /*object*/, GstDebugMessage* message,
                          gpointer /*data*/) {
    static auto logger = categoryLogger("gstreamer");

    if (level == GST_LEVEL_NONE) {
        logger->warn("GStreamer logging is disabled");
        return;
    }

    spdlog::level::level_enum logLevel = spdlog::level::off;
    if (level == GST_LEVEL_ERROR) {
        logLevel = spdlog::level::err;
    } else if (level == GST_LEVEL_WARNING || level == GST_LEVEL_FIXME) {
        logLevel = spdlog::level::warn;
    } else if (level == GST_LEVEL_INFO) {
        // GStreamer info messages are very verbose, log them on a low level.
        logLevel = spdlog::level::trace;
    }
    // Ignore lower level messages from gstreamer, because they are very verbose.

    if (!logger->should_log(logLevel)) return;

    gchar const* msg = gst_debug_message_get(message);
    logger->log(logLevel, "{}", msg);
}

void initializeGStreamer(int argc, char** argv) {
    // Set the GStreamer log level to info (those are the message that we handle in our logging function).
    std::string gstreamerLogLevel = fmt::format("{}", GST_LEVEL_INFO);
    setenv("GST_DEBUG", gstreamerLogLevel.c_str(), true);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    gst_debug_remove_log_function(gst_debug_log_default);
    gst_debug_add_log_function(gstreamerLogFunction, nullptr, nullptr);
#pragma GCC diagnostic pop

    gst_init(&argc, &argv);
}
