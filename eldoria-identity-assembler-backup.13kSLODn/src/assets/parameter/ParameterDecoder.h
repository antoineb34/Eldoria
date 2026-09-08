#pragma once

#include <cstdint>
#include <span>

#include "parameter/ParameterData.h"

namespace eld::parameter {

class ParameterDecoder {
public:
  ParameterData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::parameter
