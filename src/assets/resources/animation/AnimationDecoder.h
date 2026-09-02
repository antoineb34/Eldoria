#pragma once

#include <cstdint>
#include <span>

#include "Animation.h"
#include "AnimationAsset.h"
#include "AnimationFile.h"

namespace eld::animation {

class AnimationDecoder {
public:
    Animation decode(
        std::span<const std::uint8_t> payload
    ) const;

private:
    AnimationAsset decodeAsset(
        const AnimationFile& file
    ) const;
};

}
