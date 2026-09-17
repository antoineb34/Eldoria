#pragma once

#include "model/ModelData.h"
#include "sequence/SequenceResource.h"
#include "spot_animation/SpotAnimationData.h"

namespace eld::spot_animation {

struct SpotAnimationResource {
    SpotAnimationData data;

    const eld::model::ModelData*
        model = nullptr;

    const eld::sequence::SequenceResource*
        sequence = nullptr;
};

}
