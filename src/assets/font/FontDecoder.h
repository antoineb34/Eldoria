#pragma once

#include <cstdint>
#include <span>

#include "font/FontData.h"

namespace eld::font {

class FontDecoder {
public:
  FontData decode(std::span<const std::uint8_t> data,
                  std::span<const std::uint8_t> index) const;
};

} // namespace eld::font
