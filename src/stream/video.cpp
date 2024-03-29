#include "video.h"

#include "gstreamer/gstreamer_helpers.h"
#include "gstreamer/pipeline.h"
#include "network/ip_address.h"

#include <spdlog/fmt/fmt.h>

#include <gst/video/videooverlay.h>

#ifdef RASPBERRY_PI
// Use hardware accelerated encoders on the Raspberry Pi.
std::string const h264Encoder = "omxh264enc";
// omxh264dec currently doesn't work on Raspberry Pi 4.
// See https://www.raspberrypi.org/forums/viewtopic.php?t=252060#p1538803
std::string const h264Decoder = "avdec_h264";
#else
std::string const h264Encoder = "x264enc";
std::string const h264Decoder = "decodebin";
#endif

VideoSender::VideoSender(std::string const& device, IpAddress const& targetHost, int targetPort, int width, int height,
                         int framerate) {
    std::string videoSource = "videotestsrc";
    if (!device.empty()) {
        videoSource = fmt::format("v4l2src device={}", device);
    }

    std::string pipelineSpecification = fmt::format(
        "{} ! "
        "video/x-raw,width={},height={},framerate={}/1 ! "
        "{} ! "
        "rtph264pay config-interval=1 ! "
        "udpsink host={} port={}",
        videoSource, width, height, framerate, h264Encoder, targetHost.toString(), targetPort);
    pipeline = std::make_unique<Pipeline>(pipelineSpecification);
}

VideoSender::~VideoSender() = default;

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
        "rtpjitterbuffer ! "
        "rtph264depay ! "
        "h264parse ! "
        "{} ! "
        "videoconvert ! "
        "videoscale ! "
        "ximagesink name=sink",
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
