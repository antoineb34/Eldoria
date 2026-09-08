#pragma once

#include <optional>
#include <vector>

#include "animation/AnimationData.h"
#include "sequence/SequenceData.h"

namespace eld::sequence {

struct ResolvedAnimationFrame {
    const eld::animation::AnimationFrame* frame = nullptr;
    std::span<const eld::animation::SkeletonSlot> skeleton;

    eld::animation::AnimationFrameView view() const {
        return {
            *frame,
            skeleton
        };
    }
};


struct ResolvedSequenceFrame {
    ResolvedAnimationFrame primary;

    std::optional<
        ResolvedAnimationFrame
    > secondary;
};


struct SequenceResource : SequenceData {
    std::vector<ResolvedSequenceFrame> resolvedFrames;
};

}
