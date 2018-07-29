#pragma once

#include <memory>
#include <string>

class AudioSender {
public:
    AudioSender(std::string const& targetHost, int targetPort);
    ~AudioSender();

    void start();
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

class AudioReceiver {
public:
    AudioReceiver(int sourcePort);
    ~AudioReceiver();

    void start();
    void stop();

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
