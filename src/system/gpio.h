#pragma once

#include <mutex>

// All functions in this file only work on Raspberry Pi! On other systems they will be implemented but simply do
// nothing.

class GpioInputPin {
public:
    GpioInputPin(unsigned int pin, bool enablePullUp = false);

    bool read() const;

private:
#ifdef RASPBERRY_PI
    unsigned int _pin;
#endif
};

// A GPIO output pin, identified by the BCM pin number. If the relay flag is set, the pin can also refer to a relay on a
// relay board from Sequent Microsystems.
class GpioOutputPin {
public:
    GpioOutputPin(unsigned int pin, bool relay = false);
    ~GpioOutputPin();

    GpioOutputPin(GpioOutputPin const&) = delete;
    GpioOutputPin& operator=(GpioOutputPin const&) = delete;

    void set(bool high);

private:
#ifdef RASPBERRY_PI
    unsigned int _pin;
    bool _relay = false;

    std::unique_lock<std::mutex> lock;
#endif
};
