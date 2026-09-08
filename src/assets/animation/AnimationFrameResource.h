#pragma once

#include <span>

#include "animation/AnimationData.h"

namespace eld::animation {

struct AnimationFrameResource {
    const AnimationFrameData& frame;
    std::span<const SkeletonSlot> skeleton;
};

}
