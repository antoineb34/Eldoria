#include "parameter/ParameterAssembler.h"

#include <utility>

namespace eld::parameter {

ParameterResource ParameterAssembler::assemble(ParameterData data) const {
  return ParameterResource{
      .data = std::move(data),
  };
}

} // namespace eld::parameter
