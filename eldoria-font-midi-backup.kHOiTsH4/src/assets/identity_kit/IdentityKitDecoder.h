#pragma once

#include <cstdint>
#include <span>

#include "identity_kit/IdentityKitData.h"

namespace eld::identity_kit {

class IdentityKitDecoder {
public:
  IdentityKitData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::identity_kit
