#include "audio.h"

#include <spdlog/fmt/fmt.h>

AudioSender::AudioSender(IpAddress const& targetHost, int targetPort) {
    std::string pipelineSpecification = fmt::format(
        "pulsesrc ! "
        "queue ! "
        "audioconvert ! "
        "mulawenc ! "
        "rtppcmupay ! "
        "udpsink host={} port={}",
        targetHost.toString(), targetPort);
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

AudioReceiver::AudioReceiver(int sourcePort, double volume) {
    std::string pipelineSpecification = fmt::format(
        "udpsrc port={} caps=\"application/x-rtp\" ! "
        "rtpjitterbuffer latency=100 drop-on-latency=true ! "
        "rtppcmudepay ! "
        "mulawdec ! "
        "audioconvert ! "
        "audioresample ! "
        "volume volume={} ! "
        "pulsesink buffer-time=100000 sync=false",
        sourcePort, volume);
    pipeline = std::make_unique<Pipeline>(pipelineSpecification);
}

void AudioReceiver::start() {
    pipeline->start();
}

void AudioReceiver::stop() {
    pipeline->stop();
}
