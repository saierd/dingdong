#pragma once

#include <memory>
#include <string>

class VideoSender {
public:
    VideoSender(std::string const& targetHost, int targetPort);
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
