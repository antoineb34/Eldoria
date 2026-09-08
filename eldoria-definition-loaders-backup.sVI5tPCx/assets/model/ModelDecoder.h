#pragma once

#include <cstdint>
#include <span>

#include "model/ModelData.h"

namespace eld::model {

class ModelDecoder {
public:
    ModelData decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
