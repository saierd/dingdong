#pragma once

#include <memory>
#include <string>

#include "network/ip_address.h"

class AudioSender {
public:
    AudioSender(IpAddress const& targetHost, int targetPort, std::string const& audioSourceDevice = "");
    ~AudioSender();

    void start();
    void stop();

    bool isRunning() const;

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
