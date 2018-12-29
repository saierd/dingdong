#include "audio.h"

#include "gstreamer/gstreamer_helpers.h"
#include "util/logging.h"

AudioSender::AudioSender(IpAddress const& targetHost, int targetPort, std::string const& audioSourceDevice) {
    std::string sourceParameters;
    if (!audioSourceDevice.empty()) {
        sourceParameters = fmt::format(" device=\"{}\"", audioSourceDevice);
#ifdef RASPBERRY_PI
    } else {
        categoryLogger("gstreamer")->warn("On Raspberry Pi the audio source device must be specified explicitly");
#endif
    }

    std::string pipelineSpecification = fmt::format(
        "pulsesrc{} ! "
        "queue ! "
        "audioconvert ! "
        "mulawenc ! "
        "rtppcmupay ! "
        "udpsink host={} port={}",
        sourceParameters, targetHost.toString(), targetPort);
    pipeline = std::make_unique<Pipeline>(pipelineSpecification);
}

void AudioSender::start() {
    pipeline->start();
}

void AudioSender::stop() {
    pipeline->stop();
}

bool AudioSender::isRunning() const {
    return pipeline->isRunning();
}

AudioReceiver::AudioReceiver(int sourcePort) {
    std::string pipelineSpecification = fmt::format(
        "udpsrc port={} caps=\"application/x-rtp\" ! "
        "rtpjitterbuffer latency=100 ! "
        "rtppcmudepay ! "
        "mulawdec ! "
        "audioconvert ! "
        "audioresample ! "
        "pulsesink sync=false",
        sourcePort);
    pipeline = std::make_unique<Pipeline>(pipelineSpecification);
}

void AudioReceiver::start() {
    pipeline->start();
}

void AudioReceiver::stop() {
    pipeline->stop();
}
