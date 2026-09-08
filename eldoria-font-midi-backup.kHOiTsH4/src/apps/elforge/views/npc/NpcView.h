#pragma once

#include <optional>

#include "model/ModelData.h"
#include "model/ModelLoader.h"
#include "npc/NpcData.h"

namespace eld::elforge {

class NpcView {
public:
  std::optional<eld::model::ModelData>
  build(const eld::npc::NpcData &definition,
        const eld::model::ModelLoader &repository) const;
};

} // namespace eld::elforge
