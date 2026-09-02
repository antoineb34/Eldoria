#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "MapIndex.h"

namespace eld::map {

class MapIndexParser {
public:
    std::optional<MapIndex> parse(
        const std::vector<std::uint8_t>& bytes
    ) const;
};

}
