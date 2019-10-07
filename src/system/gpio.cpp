#include "gpio.h"

#include <spdlog/fmt/fmt.h>

#include "system/external_process.h"

// Executable for controlling GPIO on the Raspberry Pi. The -g parameter uses the original pin numbers instead of
// the number scheme used by WiringPi.
std::string const gpioExecutable = "gpio -g";

#ifdef RASPBERRY_PI
GpioOutputPin::GpioOutputPin(unsigned int pin) : _pin(pin) {
    runExternalProcess(fmt::format("{} mode {} output", gpioExecutable, _pin));
    set(false);
}
#else
GpioOutputPin::GpioOutputPin(unsigned int /*unused*/) {}
#endif

#ifdef RASPBERRY_PI
void GpioOutputPin::set(bool high) {
    runExternalProcess(fmt::format("{} write {} {}", gpioExecutable, _pin, high ? 1 : 0));
}
#else
void GpioOutputPin::set(bool /*unused*/) {}
#endif
