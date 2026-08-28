#pragma once

#include <cstdint>

#include "AnimationAsset.h"
#include "AnimationFile.h"

namespace eld::animation {

struct Animation {
    std::uint16_t id = 0;
    AnimationFile file;
    AnimationAsset asset;
};

}
