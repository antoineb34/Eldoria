#pragma once

#include <optional>

#include "definition/spot_animation/SpotAnimationDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class SpotAnimationView {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::SpotAnimationDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;

    std::optional<eld::model::Model> buildAnimationSource(
        const eld::definition::SpotAnimationDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;

    void prepareAnimatedMesh(
        const eld::definition::SpotAnimationDefinition& definition,
        eld::model::ModelMesh& mesh
    ) const;
};

}
