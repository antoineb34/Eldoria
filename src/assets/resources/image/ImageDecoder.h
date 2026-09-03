#pragma once

#include <cstdint>
#include <span>

#include "Image.h"

namespace eld::image {

class ImageDecoder {
public:
    Image decode(
        std::span<const std::uint8_t> payload
    ) const;

    Image decode(
        std::span<const std::uint8_t> dataPayload,
        std::span<const std::uint8_t> indexPayload,
        std::uint16_t frameId = 0
    ) const;
};

}
