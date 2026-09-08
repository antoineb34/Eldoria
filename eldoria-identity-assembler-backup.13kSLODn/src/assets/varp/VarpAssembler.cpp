#include "varp/VarpAssembler.h"

#include <utility>

namespace eld::varp {

VarpResource VarpAssembler::assemble(VarpData data) const {
  return VarpResource{
      .data = std::move(data),
  };
}

} // namespace eld::varp
