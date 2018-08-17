#include "audio.h"

#include "gstreamer_helpers.h"

// gst-launch-1.0 autoaudiosrc ! mulawenc ! rtppcmupay ! udpsink port=5555

class AudioSender::Impl {
public:
    GstElement* pipeline = nullptr;
    GstElement* source = nullptr;
    GstElement* encode = nullptr;
    GstElement* rtpPayload = nullptr;
    GstElement* udpSink = nullptr;

    bool isRunning = false;
};

AudioSender::AudioSender(IpAddress const& targetHost, int targetPort) {
    impl = std::make_unique<Impl>();

    impl->pipeline = gst_pipeline_new("sender");
    impl->source = gst_element_factory_make("alsasrc", nullptr);
#ifdef RASPBERRY_PI
    // On the Raspberry Pi we need to explicitly set the source device, even
    // if it is the only available microphone. Otherwise opening it will fail.
    g_object_set(g_object_cast(impl->source), "device", "plughw:1,0", nullptr);
#endif
    impl->encode = gst_element_factory_make("mulawenc", nullptr);
    impl->rtpPayload = gst_element_factory_make("rtppcmupay", nullptr);
    impl->udpSink = gst_element_factory_make("udpsink", nullptr);
    g_object_set(g_object_cast(impl->udpSink), "host", targetHost.toString().c_str(), nullptr);
    g_object_set(g_object_cast(impl->udpSink), "port", targetPort, nullptr);

    gst_bin_add_many(gst_bin_cast(impl->pipeline), impl->source, impl->encode, impl->rtpPayload, impl->udpSink, nullptr);
    gst_element_link_many(impl->source, impl->encode, impl->rtpPayload, impl->udpSink, nullptr);
}

AudioSender::~AudioSender() {
    if (impl->pipeline) gst_object_unref(gst_object_cast(impl->pipeline));
}

void AudioSender::start() {
    gst_element_set_state(impl->pipeline, GST_STATE_PLAYING);
    impl->isRunning = true;
}

void AudioSender::stop() {
    gst_element_set_state(impl->pipeline, GST_STATE_PAUSED);
    impl->isRunning = false;
}

bool AudioSender::isRunning() const {
    return impl->isRunning;
}

// gst-launch-1.0 udpsrc port=5555 caps="application/x-rtp" ! rtppcmudepay ! mulawdec ! audioconvert ! alsasink

class AudioReceiver::Impl {
public:
    GstElement* pipeline = nullptr;
    GstElement* source = nullptr;
    GstElement* rtpDepay = nullptr;
    GstElement* decode = nullptr;
    GstElement* convert = nullptr;
    GstElement* sink = nullptr;
};

AudioReceiver::AudioReceiver(int sourcePort) {
    impl = std::make_unique<Impl>();

    impl->pipeline = gst_pipeline_new("receiver");

    impl->source = gst_element_factory_make("udpsrc", nullptr);
    g_object_set(g_object_cast(impl->source), "port", sourcePort, nullptr);
    GstCaps* sourceCaps = gst_caps_new_empty_simple("application/x-rtp");
    g_object_set(g_object_cast(impl->source), "caps", sourceCaps, nullptr);

    impl->rtpDepay = gst_element_factory_make("rtppcmudepay", nullptr);
    impl->decode = gst_element_factory_make("mulawdec", nullptr);
    impl->convert = gst_element_factory_make("audioconvert", nullptr);
    impl->sink = gst_element_factory_make("alsasink", nullptr);

    gst_bin_add_many(gst_bin_cast(impl->pipeline), impl->source, impl->rtpDepay, impl->decode, impl->convert, impl->sink, nullptr);
    gst_element_link_many(impl->source, impl->rtpDepay, impl->decode, impl->convert, impl->sink, nullptr);
}

AudioReceiver::~AudioReceiver() {
    if (impl->pipeline) gst_object_unref(gst_object_cast(impl->pipeline));
}

void AudioReceiver::start() {
    gst_element_set_state(impl->pipeline, GST_STATE_PLAYING);
}

void AudioReceiver::stop() {
    gst_element_set_state(impl->pipeline, GST_STATE_PAUSED);
}
