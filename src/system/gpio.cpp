#include "gpio.h"

extern "C" {
#include <command.h>
#include <pigpiod_if2.h>
}

#include "util/logging.h"

std::string const gpioLogCategory = "gpio";

#ifdef RASPBERRY_PI
static int pigpioHandle = -1;
#endif

void checkPigpioError(int error) {
    if (error >= 0) return;

    auto logger = categoryLogger(gpioLogCategory);
    logger->error(cmdErrStr(error));
}

void initializeGpio() {
#ifdef RASPBERRY_PI
    pigpioHandle = pigpio_start(nullptr, nullptr);
    checkPigpioError(pigpioHandle);
#endif
}

void finalizeGpio() {
#ifdef RASPBERRY_PI
    pigpio_stop(pigpioHandle);
#endif
}

GpioOutputPin::GpioOutputPin(unsigned int pin) : _pin(pin) {
#ifdef RASPBERRY_PI
    set_mode(pigpioHandle, _pin, PI_OUTPUT);
#endif
}

#ifdef RASPBERRY_PI
void GpioOutputPin::set(bool high) {
    gpio_write(pigpioHandle, _pin, high ? 1 : 0);
}
#else
void GpioOutputPin::set(bool) {}
#endif
