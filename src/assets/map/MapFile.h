#pragma once

#include <cstdint>
#include <vector>

#include "binary/Compression.h"
#include "cache/Index.h"

namespace eld::map {

struct MapFile {
    std::uint16_t id = 0;
    eld::cache::IndexEntry indexEntry{};
    eld::binary::CompressionType compression =
        eld::binary::CompressionType::None;
    std::vector<std::uint8_t> bytes;
};

}
