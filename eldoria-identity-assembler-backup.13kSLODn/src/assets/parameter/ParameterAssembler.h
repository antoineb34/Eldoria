#pragma once

#include "parameter/ParameterData.h"
#include "parameter/ParameterResource.h"

namespace eld::parameter {

class ParameterAssembler {
public:
  ParameterResource assemble(ParameterData data) const;
};

} // namespace eld::parameter
