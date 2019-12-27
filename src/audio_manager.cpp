#include "audio_manager.h"

#include <mutex>

#include "audio_player.h"

class AudioManager::Impl {
public:
    Impl(Settings const& settings) : ringtonePlayer(settings.ringtoneVolume()) {}

public:
    std::mutex mutex;

    AudioPlayer ringtonePlayer;
    int numActiveCalls = 0;
};

AudioManager::AudioManager(Settings const& settings) {
    impl = std::make_unique<Impl>(settings);
}

AudioManager::~AudioManager() = default;

void AudioManager::startCall() {
    std::scoped_lock lock(impl->mutex);

    // Call is more important and stops any ringtone that might currently be playing.
    impl->ringtonePlayer.stop();

    impl->numActiveCalls++;
}

void AudioManager::stopCall() {
    std::scoped_lock lock(impl->mutex);

    impl->numActiveCalls--;
}

void AudioManager::playRingtone(std::string const& file) {
    std::scoped_lock lock(impl->mutex);

    if (impl->numActiveCalls > 0) {
        // Ignore ringtone while we have running calls.
        return;
    }

    impl->ringtonePlayer.play(file);
}
