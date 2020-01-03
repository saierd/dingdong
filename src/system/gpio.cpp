#include "gpio.h"

#include <spdlog/fmt/fmt.h>

#include <procxx/process.h>

// Script for controlling relays on the relay extension board.
std::string const relayExecutable = "./scripts/set_relay.py";

// Uses WiringPi's gpio command to control the GPIO pins on the Raspberry Pi. The -g parameter uses the original pin
// numbers instead of the number scheme used by WiringPi.
template<typename... Args>
procxx::process runGpioCommand(Args&&... args) {
    return procxx::process("gpio", "-g", std::forward<Args>(args)...);
}

#ifdef RASPBERRY_PI
// Mutex that should be locked while an output pin is set to high. This is to limit current drawn by any external
// devices that are connected to those outputs. When a GPIO action is triggered and wants to set an output to high it
// might have to wait until the last action is finished.
static std::mutex gpioOutputMutex;
#endif

#ifdef RASPBERRY_PI
GpioInputPin::GpioInputPin(unsigned int pin, bool enablePullUp) : _pin(pin) {
    runGpioCommand("mode", std::to_string(_pin), "input").exec();
    if (enablePullUp) {
        runGpioCommand("mode", std::to_string(_pin), "up").exec();
    }
}
#else
GpioInputPin::GpioInputPin(unsigned int /*unused*/, bool /*unused*/) {}
#endif

bool GpioInputPin::read() const {
#ifdef RASPBERRY_PI
    auto read = runGpioCommand("read", std::to_string(_pin));
    read.exec();

    std::string result;
    std::getline(read.output(), result);
    return (result == "1");
#else
    return false;
#endif
}

#ifdef RASPBERRY_PI
GpioOutputPin::GpioOutputPin(unsigned int pin, bool relay) : _pin(pin), _relay(relay) {
    if (!_relay) {
        runGpioCommand("mode", std::to_string(_pin), "output").exec();
    }
    set(false);
}
#else
GpioOutputPin::GpioOutputPin(unsigned int /*unused*/, bool /*unused*/) {}
#endif

GpioOutputPin::~GpioOutputPin() {
    try {
        set(false);
    } catch (...) {
    }
}

#ifdef RASPBERRY_PI
void GpioOutputPin::set(bool high) {
    // Acquire lock while pin is set to high.
    if (high && !lock.owns_lock()) {
        lock = std::unique_lock(gpioOutputMutex);
    } else if (!high && lock.owns_lock()) {
        lock.unlock();
    }

    if (!_relay) {
        runGpioCommand("write", std::to_string(_pin), std::to_string(high ? 1 : 0)).exec();
    } else {
        procxx::process(relayExecutable, std::to_string(_pin), std::to_string(high ? 1 : 0)).exec();
    }
}
#else
void GpioOutputPin::set(bool /*unused*/) {}
#endif
