#include "video.h"

#include <spdlog/fmt/fmt.h>

#include <gst/video/videooverlay.h>
#include "gstreamer/gstreamer_helpers.h"

#ifdef RASPBERRY_PI
// Use hardware accelerated encoders on the Raspberry Pi.
std::string h264Encoder = "omxh264enc";
std::string h264Decoder = "omxh264dec";
#else
std::string h264Encoder = "x264enc";
std::string h264Decoder = "decodebin";
#endif

VideoSender::VideoSender(IpAddress const& targetHost, int targetPort, int width, int height) {
    std::string pipelineSpecification = fmt::format(
        "v4l2src ! "
        "video/x-raw,width={},height={} ! "
        "videoscale ! "
        "videoconvert ! "
        "{} ! "
        "rtph264pay ! "
        "udpsink host={} port={}",
        width, height, h264Encoder, targetHost.toString(), targetPort);
    pipeline = std::make_unique<Pipeline>(pipelineSpecification);
}

void VideoSender::start() {
    pipeline->start();
}

void VideoSender::stop() {
    pipeline->stop();
}

bool VideoSender::isRunning() const {
    return pipeline->isRunning();
}

VideoReceiver::VideoReceiver(int sourcePort) {
    std::string pipelineSpecification = fmt::format(
        "udpsrc port={} caps=\"application/x-rtp\" ! "
        "rtpjitterbuffer latency=100 drop-on-latency=true ! "
        "rtph264depay ! "
        "h264parse ! "
        "{} ! "
        "videoconvert ! "
        "xvimagesink name=sink",
        sourcePort, h264Decoder);
    pipeline = std::make_unique<Pipeline>(pipelineSpecification);
}

void VideoReceiver::start() {
    pipeline->start();
}

void VideoReceiver::stop() {
    pipeline->stop();
}

void VideoReceiver::setWindowHandle(unsigned long window) {
    GstElement* sink = pipeline->getElementByName("sink");
    if (!sink) return;

    gst_video_overlay_set_window_handle(gst_video_overlay_cast(sink), window);

    if (pipeline->isRunning()) {
        stop();
        start();
    }
}
