#pragma once

#include <optional>

#include "spot_animation/SpotAnimationData.h"
#include "model/ModelData.h"
#include "model/ModelLoader.h"

namespace eld::elforge {

class SpotAnimationView {
public:
    std::optional<eld::model::ModelData> build(
        const eld::spot_animation::SpotAnimationData& definition,
        const eld::model::ModelLoader& repository
    ) const;

    std::optional<eld::model::ModelData> buildAnimationSource(
        const eld::spot_animation::SpotAnimationData& definition,
        const eld::model::ModelLoader& repository
    ) const;

    void prepareAnimatedMesh(
        const eld::spot_animation::SpotAnimationData& definition,
        eld::model::ModelData& mesh
    ) const;
};

}
