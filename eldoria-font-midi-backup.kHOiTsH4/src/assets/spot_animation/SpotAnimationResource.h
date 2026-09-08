#pragma once

#include "model/ModelResource.h"
#include "sequence/SequenceResource.h"
#include "spot_animation/SpotAnimationData.h"

namespace eld::spot_animation {

struct SpotAnimationResource {
    SpotAnimationData data;

    const eld::model::ModelResource*
        model = nullptr;

    const eld::sequence::SequenceResource*
        sequence = nullptr;
};

}
