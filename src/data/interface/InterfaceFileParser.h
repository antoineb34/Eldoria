#pragma once

#include <optional>
#include <vector>
#include <cstdint>

#include "InterfaceFile.h"

namespace eld::interface {

class InterfaceFileParser {
public:
    std::optional<InterfaceFile> parse(
        const std::vector<std::uint8_t>& payload
    ) const;
};

}
