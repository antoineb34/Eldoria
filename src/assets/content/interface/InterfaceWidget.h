#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace eld::interface {

struct InterfaceCondition {
    std::uint8_t comparison = 0;
    std::uint16_t value = 0;
};

struct InterfaceScript {
    std::vector<std::int32_t> instructions;
};

struct InterfaceChild {
    std::uint16_t id = 0;
    std::int16_t x = 0;
    std::int16_t y = 0;
};

struct InterfaceSpriteSlot {
    std::uint8_t slot = 0;
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::string sprite;
};

struct InterfaceWidget {
    std::uint16_t id = 0;
    std::optional<std::uint16_t> parentId;

    std::uint8_t type = 0;
    std::uint8_t actionType = 0;
    std::uint16_t contentType = 0;

    std::uint16_t width = 0;
    std::uint16_t height = 0;

    std::uint8_t opacity = 0;
    std::optional<std::uint16_t> hoverId;

    std::vector<InterfaceCondition> conditions;
    std::vector<InterfaceScript> scripts;

    std::uint16_t scrollHeight = 0;
    bool hidden = false;
    std::vector<InterfaceChild> children;

    bool inventorySwap = false;
    bool inventoryInterface = false;
    bool inventoryUsable = false;
    bool inventoryReplace = false;

    std::uint8_t inventoryPaddingX = 0;
    std::uint8_t inventoryPaddingY = 0;

    std::vector<std::uint16_t> itemIds;
    std::vector<std::uint16_t> itemAmounts;
    std::vector<InterfaceSpriteSlot> inventorySprites;
    std::vector<std::string> actions;

    bool filled = false;
    bool centeredText = false;
    std::uint8_t fontId = 0;
    bool textShadow = false;

    std::string text;
    std::string secondaryText;

    std::uint32_t color = 0;
    std::uint32_t secondaryColor = 0;
    std::uint32_t hoverColor = 0;
    std::uint32_t secondaryHoverColor = 0;

    std::string sprite;
    std::string secondarySprite;

    std::optional<std::uint16_t> modelId;
    std::optional<std::uint16_t> secondaryModelId;
    std::optional<std::uint16_t> animationId;
    std::optional<std::uint16_t> secondaryAnimationId;

    std::uint16_t modelZoom = 0;
    std::uint16_t modelRotationX = 0;
    std::uint16_t modelRotationY = 0;

    std::int16_t itemPaddingX = 0;
    std::int16_t itemPaddingY = 0;

    std::string selectedAction;
    std::string spellName;
    std::uint16_t spellTargets = 0;
    std::string tooltip;
};

}
