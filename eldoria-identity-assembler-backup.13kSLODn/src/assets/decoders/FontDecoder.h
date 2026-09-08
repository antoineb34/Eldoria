#pragma once

#include <cstdint>
#include <span>

#include "Font.h"

namespace eld::font {

class FontDecoder {
public:
    Font decode(
        std::span<const std::uint8_t> data,
        std::span<const std::uint8_t> index
    ) const;
};

}
