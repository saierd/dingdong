#pragma once

#include <array>

class MachineId {
private:
    MachineId(std::string const& data);
    friend MachineId getMachineId();

public:
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
