#pragma once

#include "model/ModelLoader.h"
#include "sequence/SequenceLoader.h"
#include "spot_animation/SpotAnimationData.h"
#include "spot_animation/SpotAnimationResource.h"

namespace eld::spot_animation {

class SpotAnimationAssembler {
public:
    SpotAnimationAssembler() = default;

    SpotAnimationAssembler(
        const eld::model::ModelLoader& models,
        const eld::sequence::SequenceLoader& sequences
    );

    SpotAnimationResource assemble(
        SpotAnimationData data
    ) const;

private:
    const eld::model::ModelLoader* models_ = nullptr;
    const eld::sequence::SequenceLoader* sequences_ = nullptr;
};

}
