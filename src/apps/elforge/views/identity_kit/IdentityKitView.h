#pragma once

#include <optional>

#include "definition/idk/IdentityKitDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class IdentityKitView {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::IdentityKitDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;
};

}
