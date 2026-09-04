#pragma once

#include <optional>

#include "SpotAnimation.h"
#include "Model.h"
#include "repositories/ModelRepository.h"

namespace eld::elforge {

class SpotAnimationView {
public:
    std::optional<eld::model::Model> build(
        const eld::spot_animation::SpotAnimation& definition,
        const eld::model::ModelRepository& repository
    ) const;

    std::optional<eld::model::Model> buildAnimationSource(
        const eld::spot_animation::SpotAnimation& definition,
        const eld::model::ModelRepository& repository
    ) const;

    void prepareAnimatedMesh(
        const eld::spot_animation::SpotAnimation& definition,
        eld::model::Model& mesh
    ) const;
};

}
