#include "beep.h"

#include "system/external_process.h"

std::string const beepCommand = "play -n synth .1 sine C7 vol 0.02";

void beep() {
    runExternalProcess(beepCommand);
}
