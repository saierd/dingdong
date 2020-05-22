#pragma once

#include <memory>

#include "gstreamer/pipeline.h"
#include "network/ip_address.h"

class VideoSender {
public:
    VideoSender(IpAddress const& targetHost, int targetPort, int width = 640, int height = 480, int framerate = 5);
    ~VideoSender();

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
