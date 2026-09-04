#pragma once

#include <cstdint>
#include <span>

#include "Animation.h"

namespace eld::animation {

class AnimationDecoder {
public:
    Animation decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
