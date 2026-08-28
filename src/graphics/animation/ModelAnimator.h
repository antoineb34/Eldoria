#pragma once

#include <cstddef>

#include "animation/AnimationAsset.h"
#include "model/ModelMesh.h"

namespace eld::graphics {

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
    eld::model::ModelMesh mesh;
    AnimationApplyStats stats;
};

class ModelAnimator {
public:
    AnimatedModelFrame apply(
        const eld::model::ModelMesh& source,
        const eld::animation::AnimationFrame& frame,
        const eld::animation::Skeleton& skeleton
    ) const;

    AnimationApplyStats applyInPlace(
        eld::model::ModelMesh& mesh,
        const eld::animation::AnimationFrame& frame,
        const eld::animation::Skeleton& skeleton
    ) const;
};

}
