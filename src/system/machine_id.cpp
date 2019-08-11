#include "machine_id.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>

MachineId::MachineId(std::string const& data) {
    assert(data.size() == id.size());
    std::copy_n(data.data(), id.size(), id.data());
}

std::string MachineId::toString() const {
    static_assert(sizeof(char) == sizeof(decltype(id)::value_type), "Unexpected size of number types");
    return std::string(reinterpret_cast<char const*>(id.data()), id.size());
}

MachineId getMachineId() {
    std::ifstream file("/etc/machine-id");
    std::string idString;
    file >> idString;

    return MachineId(idString);
}
