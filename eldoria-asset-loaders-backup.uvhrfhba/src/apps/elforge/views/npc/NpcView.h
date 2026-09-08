#pragma once

#include <optional>

#include "Npc.h"
#include "model/ModelData.h"
#include "model/ModelPipeline.h"

namespace eld::elforge {

class NpcView {
public:
    std::optional<eld::model::ModelData> build(
        const eld::npc::Npc& definition,
        const eld::model::ModelPipeline& repository
    ) const;
};

}
