#pragma once

#include "floor/FloorData.h"
#include "floor/FloorResource.h"

namespace eld::floor {

class FloorAssembler {
public:
  FloorResource assemble(FloorData data) const;
};

} // namespace eld::floor
