#pragma once

#include <memory>

class IpAddress;
class Pipeline;

class VideoSender {
public:
    VideoSender(std::string const& device, IpAddress const& targetHost, int targetPort, int width = 320,
                int height = 240, int framerate = 5);
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
