#pragma once

#include <string>

// This class is a workaround for a PulseAudio bug(?) on Raspberry Pi. When using a sink to play sound through
// PulseAudio, it will have a lot of latency and bad sound quality. This can be observed through `pactl list`, where
// the sink is listed with a high configured latency.
// For some reason this behavior disappears, when the sink is opened permanently, e.g. by opening pavucontrol while
// playing sound. This class opens the given sink through the PulseAudio API.
class PulseAudioSinkOccupier {
public:
    PulseAudioSinkOccupier();  // Occupies the default sink.
    explicit PulseAudioSinkOccupier(std::string const& sink);
    ~PulseAudioSinkOccupier();

private:
    struct pa_simple* handle = nullptr;
};
