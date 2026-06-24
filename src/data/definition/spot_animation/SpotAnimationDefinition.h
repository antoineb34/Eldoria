#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace eld::definition {

struct SpotAnimationDefinition {
    std::uint16_t id = 0;

    std::optional<std::uint16_t> modelId;
    std::optional<std::uint16_t> sequenceId;

    std::uint16_t scaleX = 128;
    std::uint16_t scaleY = 128;
    std::uint16_t rotation = 0;

    std::uint8_t ambient = 0;
    std::uint8_t contrast = 0;

    std::array<std::optional<std::uint16_t>, 10>
        recolorSources;

    std::array<std::optional<std::uint16_t>, 10>
        recolorDestinations;
};

}
