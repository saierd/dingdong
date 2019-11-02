#include "beep.h"

#include <procxx/process.h>

void beep() {
    procxx::process("play", "-n", "synth", ".1", "sine", "C7", "vol", "0.02").exec();
}
