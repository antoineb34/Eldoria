#pragma once

#include <cstdint>
#include <span>

#include "item/ItemData.h"

namespace eld::item {

class ItemDecoder {
public:
  ItemData decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::item
