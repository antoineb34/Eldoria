#pragma once

#include <optional>

#include "IdentityKit.h"
#include "model/ModelData.h"
#include "model/ModelLoader.h"

namespace eld::elforge {

class IdentityKitView {
public:
    std::optional<eld::model::ModelData> build(
        const eld::identity_kit::IdentityKit& definition,
        const eld::model::ModelLoader& repository
    ) const;
};

}
