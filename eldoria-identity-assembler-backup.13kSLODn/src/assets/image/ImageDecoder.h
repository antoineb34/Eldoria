#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "image/ImageData.h"

namespace eld::image {

class ImageDecoder {
public:
    ImageData decode(
        std::span<const std::uint8_t> payload
    ) const;

    ImageData decode(
        std::span<const std::uint8_t> data,
        std::span<const std::uint8_t> index,
        std::uint16_t frameId = 0
    ) const;
    std::size_t frameCount(
        std::span<const std::uint8_t> data,
        std::span<const std::uint8_t> index
    ) const;

};

}
