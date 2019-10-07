#include "screen_control.h"

#include "system/external_process.h"

std::string const enableScreenCommand = "./scripts/display/force_on.sh";
std::string const disableScreenCommand = "./scripts/display/force_off.sh";

void turnScreenOn() {
#ifdef RASPBERRY_PI
    runExternalProcess(enableScreenCommand);
#endif
}

void turnScreenOff() {
#ifdef RASPBERRY_PI
    runExternalProcess(disableScreenCommand);
#endif
}
