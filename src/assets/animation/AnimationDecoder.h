#pragma once

#include <cstdint>
#include <span>

#include "animation/AnimationData.h"

namespace eld::animation {

class AnimationDecoder {
public:
    AnimationData decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
