#pragma once

#include <optional>

#include "Npc.h"
#include "model/ModelData.h"
#include "model/ModelLoader.h"

namespace eld::elforge {

class NpcView {
public:
    std::optional<eld::model::ModelData> build(
        const eld::npc::Npc& definition,
        const eld::model::ModelLoader& repository
    ) const;
};

}
