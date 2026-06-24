#pragma once

#include <optional>

#include "definition/spot_animation/SpotAnimationDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class SpotAnimationPreviewBuilder {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::SpotAnimationDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;
};

}
