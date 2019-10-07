#pragma once

// All functions in this file only work on Raspberry Pi! On other systems they will be implemented but simply do
// nothing.

class GpioOutputPin {
public:
    GpioOutputPin(unsigned int pin);

    GpioOutputPin(GpioOutputPin const&) = delete;
    GpioOutputPin& operator=(GpioOutputPin const&) = delete;

    void set(bool high);

private:
#ifdef RASPBERRY_PI
    unsigned int _pin;
#endif
};
