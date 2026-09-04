#pragma once

#include <optional>

#include "Npc.h"
#include "Model.h"
#include "repositories/ModelRepository.h"

namespace eld::elforge {

class NpcView {
public:
    std::optional<eld::model::Model> build(
        const eld::npc::Npc& definition,
        const eld::model::ModelRepository& repository
    ) const;
};

}
