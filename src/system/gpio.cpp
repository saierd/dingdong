#include "gpio.h"

#include <spdlog/fmt/fmt.h>

#include "system/external_process.h"

// Executable for controlling GPIO on the Raspberry Pi. The -g parameter uses the original pin numbers instead of
// the number scheme used by WiringPi.
std::string const gpioExecutable = "gpio -g";

// Script for controlling relays on the relay extension board.
std::string const relayExecutable = "./scripts/set_relay.py";

#ifdef RASPBERRY_PI
GpioInputPin::GpioInputPin(unsigned int pin) : _pin(pin) {
    runExternalProcess(fmt::format("{} mode {} input", gpioExecutable, _pin));
}
#else
GpioInputPin::GpioInputPin(unsigned int /*unused*/) {}
#endif

bool GpioInputPin::read() const {
#ifdef RASPBERRY_PI
    ExternalProcess readPin(fmt::format("{} read {}", gpioExecutable, _pin));
    return (readPin.readLine() == "1");
#else
    return false;
#endif
}

#ifdef RASPBERRY_PI
GpioOutputPin::GpioOutputPin(unsigned int pin, bool relay) : _pin(pin), _relay(relay) {
    if (!_relay) {
        runExternalProcess(fmt::format("{} mode {} output", gpioExecutable, _pin));
    }
    set(false);
}
#else
GpioOutputPin::GpioOutputPin(unsigned int /*unused*/, bool /*unused*/) {}
#endif

#ifdef RASPBERRY_PI
void GpioOutputPin::set(bool high) {
    if (!_relay) {
        runExternalProcess(fmt::format("{} write {} {}", gpioExecutable, _pin, high ? 1 : 0));
    } else {
        runExternalProcess(fmt::format("{} {} {}", relayExecutable, _pin, high ? 1 : 0));
    }
}
#else
void GpioOutputPin::set(bool /*unused*/) {}
#endif
