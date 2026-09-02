#pragma once

#include <cstdint>
#include <span>

#include "Model.h"

namespace eld::model {

class ModelDecoder {
public:
    Model decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
