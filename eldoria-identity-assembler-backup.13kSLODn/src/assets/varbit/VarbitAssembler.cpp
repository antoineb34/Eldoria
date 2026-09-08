#include "varbit/VarbitAssembler.h"

#include <utility>

namespace eld::varbit {

VarbitResource VarbitAssembler::assemble(VarbitData data) const {
  return VarbitResource{
      .data = std::move(data),
  };
}

} // namespace eld::varbit
