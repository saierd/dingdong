#pragma once

#include <memory>
#include <string>

#include "gstreamer/pipeline.h"
#include "network/ip_address.h"

class AudioSender {
public:
    AudioSender(IpAddress const& targetHost, int targetPort);

    void start();
    void stop();

    bool isRunning() const;

private:
    std::unique_ptr<Pipeline> pipeline;
};

class AudioReceiver {
public:
    explicit AudioReceiver(int sourcePort, double volume = 1);

    void start();
    void stop();

private:
    std::unique_ptr<Pipeline> pipeline;
};
