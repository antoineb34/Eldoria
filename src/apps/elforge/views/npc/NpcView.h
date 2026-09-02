#pragma once

#include <optional>

#include "npc/NpcDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class NpcView {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::NpcDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;
};

}
