#pragma once

#include "identity_kit/IdentityKitData.h"
#include "identity_kit/IdentityKitResource.h"

namespace eld::identity_kit {

class IdentityKitAssembler {
public:
  IdentityKitResource assemble(IdentityKitData data) const;
};

} // namespace eld::identity_kit
