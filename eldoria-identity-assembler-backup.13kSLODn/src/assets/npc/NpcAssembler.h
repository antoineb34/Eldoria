#pragma once

#include "npc/NpcData.h"
#include "npc/NpcResource.h"

namespace eld::npc {

class NpcAssembler {
public:
  NpcResource assemble(NpcData data) const;
};

} // namespace eld::npc
