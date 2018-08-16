#pragma once

#include <memory>
#include <string>

#include "network/ip_address.h"

class VideoSender {
public:
    VideoSender(IpAddress const& targetHost, int targetPort);
    ~VideoSender();

    void start();
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

class VideoReceiver {
public:
    VideoReceiver(int sourcePort);
    ~VideoReceiver();

    void start();
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
