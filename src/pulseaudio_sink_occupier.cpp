#include "pulseaudio_sink_occupier.h"

#include <stdexcept>

#include <pulse/error.h>
#include <pulse/simple.h>

PulseAudioSinkOccupier::PulseAudioSinkOccupier() : PulseAudioSinkOccupier("") {}

PulseAudioSinkOccupier::PulseAudioSinkOccupier(const std::string& sink) {
    pa_sample_spec sampleFormat;
    sampleFormat.format = PA_SAMPLE_S16NE;
    sampleFormat.channels = 2;
    sampleFormat.rate = 44100;

    int error = 0;
    handle = pa_simple_new(nullptr,                                // Default server.
                           "",                                     // Application name.
                           PA_STREAM_PLAYBACK,                     // Stream direction.
                           sink.empty() ? nullptr : sink.c_str(),  // Device name or nullptr for the default sink.
                           "Occupier",                             // Stream description.
                           &sampleFormat,                          // Sample format.
                           nullptr,                                // Use default channel map
                           nullptr,                                // Use default buffering attributes.
                           &error);

    if (handle == nullptr) {
        throw std::runtime_error(pa_strerror(error));
    }
}

PulseAudioSinkOccupier::~PulseAudioSinkOccupier() {
    if (handle) {
        pa_simple_free(handle);
    }
}
