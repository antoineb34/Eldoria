#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace eld::identity_kit {

struct IdentityKit {
    std::uint16_t id = 0;
    std::optional<std::uint8_t> bodyPartId;
    bool selectable = true;

    std::vector<std::uint16_t> modelIds;

    std::array<std::optional<std::uint16_t>, 10> recolorSources;
    std::array<std::optional<std::uint16_t>, 10> recolorDestinations;
    std::array<std::optional<std::uint16_t>, 5> headModelIds;
};

}
