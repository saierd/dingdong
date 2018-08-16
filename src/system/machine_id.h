#pragma once

#include <array>

class MachineId {
public:
    MachineId(std::string const& data);

    std::string toString() const {
        return std::string(id.data(), id.size());
    }

    bool operator==(MachineId const& other) const {
        return id == other.id;
    }

    bool operator!=(MachineId const& other) const {
        return !(*this == other);
    }

private:
    std::array<char, 32> id;
};

MachineId getMachineId();
