#pragma once

#include <optional>

#include "IdentityKit.h"
#include "Model.h"
#include "repositories/ModelRepository.h"

namespace eld::elforge {

class IdentityKitView {
public:
    std::optional<eld::model::Model> build(
        const eld::identity_kit::IdentityKit& definition,
        const eld::model::ModelRepository& repository
    ) const;
};

}
