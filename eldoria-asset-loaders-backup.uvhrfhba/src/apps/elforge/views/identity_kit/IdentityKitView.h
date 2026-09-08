#pragma once

#include <optional>

#include "IdentityKit.h"
#include "model/ModelData.h"
#include "model/ModelPipeline.h"

namespace eld::elforge {

class IdentityKitView {
public:
    std::optional<eld::model::ModelData> build(
        const eld::identity_kit::IdentityKit& definition,
        const eld::model::ModelPipeline& repository
    ) const;
};

}
