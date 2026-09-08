#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "Widget.h"

namespace eld::interface {

class WidgetDecoder {
public:
    std::vector<Widget> decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
