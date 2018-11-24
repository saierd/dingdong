#include "audio.h"

#include "gstreamer/gstreamer_helpers.h"
#include "util/logging.h"

class AudioSender::Impl {
public:
    GstElement* pipeline = nullptr;
    bool isRunning = false;
};

AudioSender::AudioSender(IpAddress const& targetHost, int targetPort, std::string const& audioSourceDevice) {
    impl = std::make_unique<Impl>();

    std::string sourceParameters;
    if (!audioSourceDevice.empty()) {
        sourceParameters = fmt::format(" device=\"{}\"", audioSourceDevice);
#ifdef RASPBERRY_PI
    } else {
        categoryLogger("gstreamer")->warn("On Raspberry Pi the audio source device must be specified explicitly");
#endif
    }

    std::string pipeline = fmt::format(
        "pulsesrc{} ! "
        "queue ! "
        "audioconvert ! "
        "mulawenc ! "
        "rtppcmupay ! "
        "udpsink host={} port={}",
        sourceParameters, targetHost.toString(), targetPort);
    impl->pipeline = runGStreamerPipeline(pipeline);
}

AudioSender::~AudioSender() {
    if (impl->pipeline) {
        gst_element_set_state(impl->pipeline, GST_STATE_NULL);
        gst_object_unref(gst_object_cast(impl->pipeline));
    }
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

class AudioReceiver::Impl {
public:
    GstElement* pipeline = nullptr;
};

AudioReceiver::AudioReceiver(int sourcePort) {
    impl = std::make_unique<Impl>();

    std::string pipeline = fmt::format(
        "udpsrc port={} caps=\"application/x-rtp\" ! "
        "rtpjitterbuffer latency=100 ! "
        "rtppcmudepay ! "
        "mulawdec ! "
        "audioconvert ! "
        "audioresample ! "
        "pulsesink sync=false",
        sourcePort);
    impl->pipeline = runGStreamerPipeline(pipeline);
}

AudioReceiver::~AudioReceiver() {
    if (impl->pipeline) {
        gst_element_set_state(impl->pipeline, GST_STATE_NULL);
        gst_object_unref(gst_object_cast(impl->pipeline));
    }
}

void AudioReceiver::start() {
    gst_element_set_state(impl->pipeline, GST_STATE_PLAYING);
}

void AudioReceiver::stop() {
    gst_element_set_state(impl->pipeline, GST_STATE_PAUSED);
}
