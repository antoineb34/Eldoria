#pragma once

#include "varp/VarpData.h"
#include "varp/VarpResource.h"

namespace eld::varp {

class VarpAssembler {
public:
  VarpResource assemble(VarpData data) const;
};

} // namespace eld::varp
