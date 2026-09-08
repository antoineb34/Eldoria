#pragma once

#include <cstdint>
#include <span>

#include "varbit/VarbitData.h"

namespace eld::varbit {

class VarbitDecoder {
public:
  VarbitData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::varbit
