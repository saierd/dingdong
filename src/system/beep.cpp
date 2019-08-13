#include "beep.h"

#include "system/external_process.h"

std::string const beepCommand = "play -n synth .1 sine C7";

void beep() {
    ExternalProcess beep(beepCommand);
}
