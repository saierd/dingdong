#include "screen_control.h"

#include <procxx/process.h>

std::string const enableScreenCommand = "./scripts/display/force_on.sh";
std::string const disableScreenCommand = "./scripts/display/force_off.sh";

void turnScreenOn() {
#ifdef RASPBERRY_PI
    procxx::process(enableScreenCommand).exec();
#endif
}

void turnScreenOff() {
#ifdef RASPBERRY_PI
    procxx::process(disableScreenCommand).exec();
#endif
}
