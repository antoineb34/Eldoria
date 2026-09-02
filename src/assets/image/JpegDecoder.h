#pragma once

#include <cstdint>
#include <vector>

#include "Image.h"

namespace eld::image {

class JpegDecoder {
public:
    Image decode(
        const std::vector<std::uint8_t>& bytes
    ) const;
};

}
