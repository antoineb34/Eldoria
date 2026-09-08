#pragma once

#include <optional>

#include "Location.h"
#include "model/ModelData.h"
#include "model/ModelLoader.h"

namespace eld::elforge {

class LocationView {
public:
    std::optional<eld::model::ModelData> build(
        const eld::location::Location& definition,
        const eld::model::ModelLoader& repository
    ) const;

    std::optional<eld::model::ModelData> buildAnimationSource(
        const eld::location::Location& definition,
        const eld::model::ModelLoader& repository
    ) const;

    void prepareAnimatedMesh(
        const eld::location::Location& definition,
        eld::model::ModelData& mesh
    ) const;
};

}
