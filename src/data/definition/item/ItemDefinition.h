#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace eld::definition {

struct ItemRecolor {
    std::uint16_t source = 0;
    std::uint16_t destination = 0;
};

struct ItemStackVariant {
    std::optional<std::uint16_t> itemId;
    std::uint16_t amount = 0;
};

struct ItemDefinition {
    std::uint16_t id = 0;

    std::string name = "null";
    std::string description;

    std::optional<std::uint16_t> inventoryModelId;

    std::uint16_t zoom = 2000;
    std::uint16_t rotationX = 0;
    std::uint16_t rotationY = 0;
    std::uint16_t rotationZ = 0;
    std::int16_t offsetX = 0;
    std::int16_t offsetY = 0;

    bool stackable = false;
    std::uint32_t value = 1;
    bool membersOnly = false;

    std::array<std::string, 5> groundActions;
    std::array<std::string, 5> inventoryActions;

    std::vector<ItemRecolor> recolors;

    std::array<std::optional<std::uint16_t>, 3>
        maleModelIds;

    std::array<std::optional<std::uint16_t>, 3>
        femaleModelIds;

    std::int8_t maleModelOffset = 0;
    std::int8_t femaleModelOffset = 0;

    std::array<std::optional<std::uint16_t>, 2>
        maleHeadModelIds;

    std::array<std::optional<std::uint16_t>, 2>
        femaleHeadModelIds;

    std::optional<std::uint16_t> noteItemId;
    std::optional<std::uint16_t> noteTemplateId;

    std::array<ItemStackVariant, 10> stackVariants;

    std::uint16_t scaleX = 128;
    std::uint16_t scaleY = 128;
    std::uint16_t scaleZ = 128;

    std::int8_t ambient = 0;
    std::int8_t contrast = 0;
    std::uint8_t team = 0;
};

}
