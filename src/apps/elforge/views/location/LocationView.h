#pragma once

#include <optional>

#include "Location.h"
#include "Model.h"
#include "repositories/ModelRepository.h"

namespace eld::elforge {

class LocationView {
public:
    std::optional<eld::model::Model> build(
        const eld::location::Location& definition,
        const eld::model::ModelRepository& repository
    ) const;

    std::optional<eld::model::Model> buildAnimationSource(
        const eld::location::Location& definition,
        const eld::model::ModelRepository& repository
    ) const;

    void prepareAnimatedMesh(
        const eld::location::Location& definition,
        eld::model::Model& mesh
    ) const;
};

}
