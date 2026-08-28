#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "AnimationFile.h"

namespace eld::animation {

class AnimationFileParser {
public:
    std::optional<AnimationFile> parse(
        const std::vector<std::uint8_t>& bytes
    ) const;
};

}
