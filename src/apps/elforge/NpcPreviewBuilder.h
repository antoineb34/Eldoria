#pragma once

#include <optional>

#include "definition/npc/NpcDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class NpcPreviewBuilder {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::NpcDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;
};

}
