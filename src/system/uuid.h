#pragma once

#include <array>
#include <string>

class UUID {
public:
    UUID();
    explicit UUID(std::string const& uuid);

    std::string toString() const;

    bool operator==(UUID const& other) const {
        return id == other.id;
    }

    bool operator!=(UUID const& other) const {
        return !(*this == other);
    }

private:
    std::array<uint8_t, 16> id;
};
