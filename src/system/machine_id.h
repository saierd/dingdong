#pragma once

#include <array>
#include <string>

class MachineId {
public:
    explicit MachineId(std::string const& data);

    std::string toString() const;

    bool operator==(MachineId const& other) const {
        return id == other.id;
    }

    bool operator!=(MachineId const& other) const {
        return !(*this == other);
    }

private:
    std::array<uint8_t, 32> id;
};

MachineId getMachineId();
