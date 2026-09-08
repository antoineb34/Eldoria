#include "npc/NpcAssembler.h"

#include <utility>

namespace eld::npc {

NpcResource NpcAssembler::assemble(NpcData data) const {
  return NpcResource{
      .data = std::move(data),
  };
}

} // namespace eld::npc
