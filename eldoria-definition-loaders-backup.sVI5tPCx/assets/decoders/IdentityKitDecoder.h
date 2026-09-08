#pragma once

#include <cstdint>
#include <span>

#include "IdentityKit.h"

namespace eld::identity_kit {

class IdentityKitDecoder {
public:
    IdentityKit decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
