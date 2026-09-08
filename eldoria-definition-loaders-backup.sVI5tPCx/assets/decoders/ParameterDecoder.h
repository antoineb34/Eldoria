#pragma once

#include <cstdint>
#include <span>

#include "Parameter.h"

namespace eld::parameter {

class ParameterDecoder {
public:
    Parameter decode(
        std::span<const std::uint8_t> payload
    ) const;
};

}
