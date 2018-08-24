#include "audio_player.h"

#include "gstreamer/gstreamer_helpers.h"

class AudioPlayer::Impl {
public:
    GstElement* pipeline = nullptr;
    GstElement* source = nullptr;
    GstElement* decoder = nullptr;
    GstElement* convert = nullptr;
    GstElement* sink = nullptr;
};

AudioPlayer::AudioPlayer() {
    impl = std::make_unique<Impl>();

    impl->pipeline = gst_pipeline_new("audio_player");
    impl->source = gst_element_factory_make("filesrc", nullptr);
    impl->decoder = gst_element_factory_make("wavparse", nullptr);
    impl->convert = gst_element_factory_make("audioconvert", nullptr);
    impl->sink = gst_element_factory_make("alsasink", nullptr);
    gst_bin_add_many(gst_bin_cast(impl->pipeline), impl->source, impl->decoder, impl->convert, impl->sink, nullptr);
    gst_element_link(impl->source, impl->decoder);
    gst_element_link(impl->convert, impl->sink);
    gst_element_link(impl->decoder, impl->convert);
}

AudioPlayer::~AudioPlayer() {
    if (impl->pipeline) gst_object_unref(gst_object_cast(impl->pipeline));
}

void AudioPlayer::play(std::string const& file) {
    gst_element_set_state(impl->pipeline, GST_STATE_READY);
    if (!file.empty()) {
        g_object_set(g_object_cast(impl->source), "location", file.c_str(), nullptr);
        gst_element_set_state(impl->pipeline, GST_STATE_PLAYING);
    }
}