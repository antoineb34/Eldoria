#pragma once

#include <cstdint>
#include <span>

#include "spot_animation/SpotAnimationData.h"

namespace eld::spot_animation {

class SpotAnimationDecoder {
public:
    SpotAnimationData decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
