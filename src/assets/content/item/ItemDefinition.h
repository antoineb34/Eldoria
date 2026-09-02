#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace eld::definition {

enum class ItemWearPosition : std::uint8_t {
    Hat = 0,
    Back = 1,
    Front = 2,
    RightHand = 3,
    Torso = 4,
    LeftHand = 5,
    Arms = 6,
    Legs = 7,
    Head = 8,
    Hands = 9,
    Feet = 10,
    Jaw = 11,
    Ring = 12,
    Quiver = 13
};

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

    // Server-side/cache variants that expose wear-position opcodes use these
    // to describe the appearance slot occupied by the item and any body slots
    // it suppresses. The classic 317 client obj.dat does not always contain
    // these fields, so equipment preview also has a conservative fallback.
    std::optional<ItemWearPosition> wearPosition;
    std::optional<ItemWearPosition> wearPosition2;
    std::optional<ItemWearPosition> wearPosition3;

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
