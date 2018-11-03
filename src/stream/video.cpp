#include "video.h"

#include "gstreamer/gstreamer_helpers.h"

// gst-launch-1.0 -v v4l2src ! videoscale ! videoconvert ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay ! udpsink port=5556

class VideoSender::Impl {
public:
    GstElement* pipeline = nullptr;
    GstElement* source = nullptr;
    GstElement* scale = nullptr;
    GstElement* convert = nullptr;
    GstElement* encode = nullptr;
    GstElement* rtpPayload = nullptr;
    GstElement* udpSink = nullptr;
};

VideoSender::VideoSender(IpAddress const& targetHost, int targetPort) {
    impl = std::make_unique<Impl>();

    impl->pipeline = gst_pipeline_new("videosender");
    impl->source = gst_element_factory_make("v4l2src", nullptr);
    impl->scale = gst_element_factory_make("videoscale", nullptr);
    impl->convert = gst_element_factory_make("videoconvert", nullptr);
    impl->encode = gst_element_factory_make("x264enc", nullptr);
    g_object_set(g_object_cast(impl->encode), "tune", 4, nullptr); // zerolatency
    g_object_set(g_object_cast(impl->encode), "bitrate", 500, nullptr);
    g_object_set(g_object_cast(impl->encode), "speed-preset", 2, nullptr); // superfast
    impl->rtpPayload = gst_element_factory_make("rtph264pay", nullptr);
    impl->udpSink = gst_element_factory_make("udpsink", nullptr);
    g_object_set(g_object_cast(impl->udpSink), "host", targetHost.toString().c_str(), nullptr);
    g_object_set(g_object_cast(impl->udpSink), "port", targetPort, nullptr);

    gst_bin_add_many(gst_bin_cast(impl->pipeline), impl->source, impl->scale, impl->convert, impl->encode, impl->rtpPayload, impl->udpSink, nullptr);
    gst_element_link_many(impl->source, impl->scale, impl->convert, impl->encode, impl->rtpPayload, impl->udpSink, nullptr);
}

VideoSender::~VideoSender() {
    if (impl->pipeline) {
        gst_element_set_state(impl->pipeline, GST_STATE_NULL);
        gst_object_unref(gst_object_cast(impl->pipeline));
    }
}

void VideoSender::start() {
    gst_element_set_state(impl->pipeline, GST_STATE_PLAYING);
}

void VideoSender::stop() {
    gst_element_set_state(impl->pipeline, GST_STATE_PAUSED);
}


// gst-launch-1.0 -v udpsrc port=5556 caps = "application/x-rtp" ! rtph264depay ! decodebin ! videoconvert ! autovideosink

class VideoReceiver::Impl {
public:
    GstElement* pipeline = nullptr;
    GstElement* source = nullptr;
    GstElement* rtpDepay = nullptr;
    GstElement* decode = nullptr;
    GstElement* convert = nullptr;
    GstElement* sink = nullptr;
};

VideoReceiver::VideoReceiver(int sourcePort) {
    impl = std::make_unique<Impl>();

    impl->pipeline = gst_pipeline_new("videoreceiver");
    impl->source = gst_element_factory_make("udpsrc", nullptr);
    g_object_set(g_object_cast(impl->source), "port", sourcePort, nullptr);
    GstCaps* sourceCaps = gst_caps_new_empty_simple("application/x-rtp");
    g_object_set(g_object_cast(impl->source), "caps", sourceCaps, nullptr);

    impl->rtpDepay = gst_element_factory_make("rtph264depay", nullptr);
    impl->decode = gst_element_factory_make("avdec_h264", nullptr);
    impl->convert = gst_element_factory_make("videoconvert", nullptr);
    impl->sink = gst_element_factory_make("autovideosink", nullptr);

    gst_bin_add_many(gst_bin_cast(impl->pipeline), impl->source, impl->rtpDepay, impl->decode, impl->convert, impl->sink, nullptr);
    gst_element_link_many(impl->source, impl->rtpDepay, impl->decode, impl->convert, impl->sink, nullptr);
}

VideoReceiver::~VideoReceiver() {
    if (impl->pipeline) {
        gst_element_set_state(impl->pipeline, GST_STATE_NULL);
        gst_object_unref(gst_object_cast(impl->pipeline));
    }
}

void VideoReceiver::start() {
    gst_element_set_state(impl->pipeline, GST_STATE_PLAYING);
}

void VideoReceiver::stop() {
    gst_element_set_state(impl->pipeline, GST_STATE_PAUSED);
}
