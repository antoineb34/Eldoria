#pragma once

#include <cstdint>
#include <span>

#include "Location.h"

namespace eld::location {

class LocationDecoder {
public:
    Location decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
