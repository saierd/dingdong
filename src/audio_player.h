#pragma once

#include <memory>
#include <string>

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    void play(std::string const& file);

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
