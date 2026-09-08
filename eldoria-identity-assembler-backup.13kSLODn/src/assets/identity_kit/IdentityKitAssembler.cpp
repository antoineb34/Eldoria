#include "identity_kit/IdentityKitAssembler.h"

#include <utility>

namespace eld::identity_kit {

IdentityKitResource IdentityKitAssembler::assemble(IdentityKitData data) const {
  return IdentityKitResource{
      .data = std::move(data),
  };
}

} // namespace eld::identity_kit
