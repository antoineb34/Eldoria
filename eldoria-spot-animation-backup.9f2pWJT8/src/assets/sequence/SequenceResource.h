#pragma once

#include <optional>
#include <span>
#include <vector>

#include "animation/AnimationFrameResource.h"
#include "sequence/SequenceData.h"

namespace eld::sequence {

struct AnimationFrameHandle {
    // Borrows immutable data. The animation pipeline must outlive this handle.
    const eld::animation::AnimationFrameData* frame = nullptr;
    std::span<const eld::animation::SkeletonSlot> skeleton;

    eld::animation::AnimationFrameResource resource() const {
        return {
            *frame,
            skeleton
        };
    }
};


struct ResolvedSequenceFrame {
    AnimationFrameHandle primary;

    std::optional<
        AnimationFrameHandle
    > secondary;
};


struct SequenceResource {
    SequenceData data;
    std::vector<ResolvedSequenceFrame> resolvedFrames;
};

}
