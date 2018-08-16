#include "uuid.h"

#include <uuid/uuid.h>

UUID::UUID() {
    uuid_generate(id.data());
}

UUID::UUID(std::string const& uuid) {
    uuid_parse(uuid.c_str(), id.data());
}

std::string UUID::toString() const {
    std::array<char, 37> string;
    uuid_unparse(id.data(), string.data());

    return string.data();
}
