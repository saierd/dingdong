#pragma once

#include <memory>

#include "settings.h"

class AudioManager {
public:
    AudioManager(Settings const& settings);
    ~AudioManager();

    void startCall();
    void stopCall();

    void playRingtone(std::string const& file);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
