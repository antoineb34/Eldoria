#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "interface/WidgetData.h"

namespace eld::interface {

class WidgetDecoder {
public:
  std::vector<WidgetData> decode(std::span<const std::uint8_t> payload) const;
};

} // namespace eld::interface
