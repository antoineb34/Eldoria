#pragma once

#include <optional>

#include "definition/location/LocationDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class LocationView {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::LocationDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;

    std::optional<eld::model::Model> buildAnimationSource(
        const eld::definition::LocationDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;

    void prepareAnimatedMesh(
        const eld::definition::LocationDefinition& definition,
        eld::model::ModelMesh& mesh
    ) const;
};

}
