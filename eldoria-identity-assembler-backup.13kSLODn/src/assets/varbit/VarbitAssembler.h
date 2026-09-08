#pragma once

#include "varbit/VarbitData.h"
#include "varbit/VarbitResource.h"

namespace eld::varbit {

class VarbitAssembler {
public:
  VarbitResource assemble(VarbitData data) const;
};

} // namespace eld::varbit
