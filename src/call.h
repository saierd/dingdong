#pragma once

#include <memory>

#include "instance.h"
#include "settings.h"
#include "system/uuid.h"

class Call {
public:
    // Create a call. Note that this will not send anything yet, but it will
    // start the receiver thread on our side on a free port.
    Call(UUID const& id, Instance const& target);
    Call(Instance const& target);
    ~Call();

    Call(Call const&) = delete;
    Call& operator=(Call const&) = delete;
    Call(Call&& other);
    Call& operator=(Call&& other);

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

    bool isInvalid() const;
    void invalidate();

protected:
    class Impl;
    std::unique_ptr<Impl> impl;
};
