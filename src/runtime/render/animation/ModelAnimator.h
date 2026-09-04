#pragma once

#include <cstddef>
#include <span>

#include "Animation.h"
#include "Model.h"

namespace eld::render {

struct AnimationApplyStats {
    std::size_t implicitPivots = 0;
    std::size_t explicitTransforms = 0;
    std::size_t invalidSkeletonSlots = 0;

    std::size_t translatedVertices = 0;
    std::size_t rotatedVertices = 0;
    std::size_t scaledVertices = 0;
    std::size_t alphaFaces = 0;

    std::size_t ignoredUnknownType4 = 0;
};

struct AnimatedModelFrame {
    eld::model::Model mesh;
    AnimationApplyStats stats;
};

class ModelAnimator {
public:
    AnimatedModelFrame apply(
        const eld::model::Model& source,
        const eld::animation::AnimationFrame& frame,
        std::span<const eld::animation::SkeletonSlot> skeleton
    ) const;

    AnimationApplyStats applyInPlace(
        eld::model::Model& mesh,
        const eld::animation::AnimationFrame& frame,
        std::span<const eld::animation::SkeletonSlot> skeleton
    ) const;
};

}
