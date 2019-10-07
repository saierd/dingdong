#pragma once

// All functions in this file only work on Raspberry Pi! On other systems they will be implemented but simply do
// nothing.

class GpioInputPin {
public:
    GpioInputPin(unsigned int pin);

    bool read() const;

private:
#ifdef RASPBERRY_PI
    unsigned int _pin;
#endif
};

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
