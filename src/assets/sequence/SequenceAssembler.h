#pragma once

#include "animation/AnimationFrameTable.h"
#include "sequence/SequenceData.h"
#include "sequence/SequenceResource.h"

namespace eld::sequence {

class SequenceAssembler {
public:
    SequenceAssembler() = default;

    explicit SequenceAssembler(
        const eld::animation::AnimationFrameTable& frames
    );

    SequenceResource assemble(
        SequenceData data
    ) const;

private:
    const eld::animation::AnimationFrameTable*
        frames_ = nullptr;
};

}
