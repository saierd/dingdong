#pragma once

#include <memory>

#include <gtkmm/widget.h>

#include "instance.h"
#include "settings.h"
#include "system/uuid.h"

class AudioManager;
class VideoReceiver;

class Call {
public:
    // Create a call. Note that this will not send anything yet, but it will
    // start the receiver thread on our side on a free port.
    Call(Settings const& self, UUID const& id, Instance const& target, std::shared_ptr<AudioManager> audioManager);
    Call(Settings const& self, Instance const& target, std::shared_ptr<AudioManager> audioManager);
    ~Call();

    Call(Call const&) = delete;
    Call& operator=(Call const&) = delete;
    Call(Call&& other) noexcept;
    Call& operator=(Call&& other) noexcept;

    UUID id() const;
    Instance target() const;

    // The port on which the receiver on our side listens.
    int receiverPort() const;

    // Create the sender thread that will send to the given port at the target
    // instance. This will not start sending yet, call the start method to do
    // that.
    void connect(int senderPort);

    void start();
    void stop();

    void mute();
    void unmute();

    bool isRunning() const;
    bool isMuted() const;

    void setRemoteSendsVideo(bool remoteSendsVideo);
    bool remoteSendsVideo() const;

    int videoReceiverPort() const;
    std::shared_ptr<VideoReceiver> videoReceiver() const;

    void connectVideo(int senderPort);
    void startVideo();
    void stopVideo();
    bool canSendVideo() const;
    bool isSendingVideo() const;

    bool isInvalid() const;
    void invalidate();

protected:
    class Impl;
    std::unique_ptr<Impl> impl;
};
