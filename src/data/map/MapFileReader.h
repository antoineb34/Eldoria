#pragma once

#include <cstdint>

#include "MapFile.h"
#include "cache/Store.h"

namespace eld::map {

class MapFileReader {
public:
    MapFile read(
        const eld::cache::Store& store,
        std::uint16_t fileId
    ) const;
};

}
