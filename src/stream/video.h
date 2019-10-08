#pragma once

#include <memory>

#include "gstreamer/pipeline.h"
#include "network/ip_address.h"

class VideoSender {
public:
    VideoSender(IpAddress const& targetHost, int targetPort, int width = 320, int height = 240);

    void start();
    void stop();

    bool isRunning() const;

private:
    std::unique_ptr<Pipeline> pipeline;
};

class VideoReceiver {
public:
    explicit VideoReceiver(int sourcePort);

    void start();
    void stop();

    void setWindowHandle(unsigned long window);

private:
    std::unique_ptr<Pipeline> pipeline;
};
