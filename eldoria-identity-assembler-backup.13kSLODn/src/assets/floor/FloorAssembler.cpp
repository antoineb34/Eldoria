#include "floor/FloorAssembler.h"

#include <utility>

namespace eld::floor {

FloorResource FloorAssembler::assemble(FloorData data) const {
  return FloorResource{
      .data = std::move(data),
  };
}

} // namespace eld::floor
