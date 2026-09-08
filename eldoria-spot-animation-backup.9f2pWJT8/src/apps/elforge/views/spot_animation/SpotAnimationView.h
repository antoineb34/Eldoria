#pragma once

#include <optional>

#include "SpotAnimation.h"
#include "model/ModelData.h"
#include "model/ModelLoader.h"

namespace eld::elforge {

class SpotAnimationView {
public:
    std::optional<eld::model::ModelData> build(
        const eld::spot_animation::SpotAnimation& definition,
        const eld::model::ModelLoader& repository
    ) const;

    std::optional<eld::model::ModelData> buildAnimationSource(
        const eld::spot_animation::SpotAnimation& definition,
        const eld::model::ModelLoader& repository
    ) const;

    void prepareAnimatedMesh(
        const eld::spot_animation::SpotAnimation& definition,
        eld::model::ModelData& mesh
    ) const;
};

}
