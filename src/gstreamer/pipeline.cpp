#include "pipeline.h"

#include "gstreamer_helpers.h"
#include "util/logging.h"

Pipeline::Pipeline(std::string const& specification) {
    GError* error = nullptr;
    pipeline = gst_parse_launch(specification.c_str(), &error);
    if (error != nullptr) {
        categoryLogger("gstreamer")->error(error->message);
    }
}

Pipeline::~Pipeline() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(gst_object_cast(pipeline));
    }
}

void Pipeline::start() {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    running = true;
}

void Pipeline::stop() {
    gst_element_set_state(pipeline, GST_STATE_PAUSED);
    running = false;
}

bool Pipeline::isRunning() const {
    return running;
}
