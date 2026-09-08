#pragma once

#include <cstdint>
#include <span>

#include "map/MapIndex.h"

namespace eld::map {

class MapIndexDecoder {
public:
    MapIndex decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
