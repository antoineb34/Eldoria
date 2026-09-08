#pragma once

#include <cstdint>
#include <span>

#include "Item.h"

namespace eld::item {

class ItemDecoder {
public:
    Item decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
