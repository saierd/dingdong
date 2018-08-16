#pragma once

#include <array>
#include <string>

class UUID {
public:
    UUID();
    UUID(std::string const& uuid);

    std::string toString() const;

    bool operator==(UUID const& other) const {
        return id == other.id;
    }

    bool operator!=(UUID const& other) const {
        return !(*this == other);
    }

private:
    std::array<unsigned char, 16> id;
};
