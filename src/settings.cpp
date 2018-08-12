#include "settings.h"

Settings::Settings(std::string name) : Instance(getMachineId(), std::move(name)) {}
