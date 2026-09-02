#pragma once

#include <cstdint>
#include <vector>

#include "AnimationAsset.h"

namespace eld::animation {

struct Animation {
    std::uint16_t id = 0;
    std::vector<std::uint8_t> bytes;
    AnimationAsset asset;
};

}
