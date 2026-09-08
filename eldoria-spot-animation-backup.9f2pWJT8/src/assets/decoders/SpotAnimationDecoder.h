#pragma once

#include <cstdint>
#include <span>

#include "SpotAnimation.h"

namespace eld::spot_animation {

class SpotAnimationDecoder {
public:
    SpotAnimation decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
