#pragma once

#include "animation/AnimationPipeline.h"
#include "sequence/SequenceData.h"
#include "sequence/SequenceResource.h"

namespace eld::sequence {

class SequenceAssembler {
public:
    SequenceResource assemble(
        SequenceData data,
        const eld::animation::AnimationPipeline& animations
    ) const;
};

}
