#include "machine_id.h"

#include <cassert>
#include <cstring>
#include <fstream>

MachineId::MachineId(std::string const& data) {
    assert(data.size() == id.size());
    std::strncpy(id.data(), data.c_str(), id.size());
}

MachineId getMachineId() {
    std::ifstream file("/etc/machine-id");
    std::string idString;
    file >> idString;

    return MachineId(idString);
}
