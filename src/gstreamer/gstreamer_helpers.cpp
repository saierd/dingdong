#include "gstreamer_helpers.h"

#include "util/logging.h"

GstElement* runGStreamerPipeline(std::string const& pipeline) {
    GError* error = nullptr;
    GstElement* pipelineElement = gst_parse_launch(pipeline.c_str(), &error);
    if (error != nullptr) {
        categoryLogger("gstreamer")->error(error->message);
    }
    return pipelineElement;
}
