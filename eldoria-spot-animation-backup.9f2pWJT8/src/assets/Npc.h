#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace eld::npc {

struct NpcRecolor {
    std::uint16_t source = 0;
    std::uint16_t destination = 0;
};

struct Npc {
    std::uint16_t id = 0;

    std::string name = "null";
    std::string description;

    std::vector<std::uint16_t> modelIds;
    std::vector<std::uint16_t> headModelIds;
    std::vector<NpcRecolor> recolors;

    std::int8_t size = 1;

    std::optional<std::uint16_t> idleAnimationId;
    std::optional<std::uint16_t> walkAnimationId;
    std::optional<std::uint16_t> turnAroundAnimationId;
    std::optional<std::uint16_t> turnRightAnimationId;
    std::optional<std::uint16_t> turnLeftAnimationId;

    std::array<std::string, 5> actions;

    bool visibleOnMinimap = true;
    bool renderPriority = false;
    bool clickable = true;

    std::optional<std::uint16_t> combatLevel;
    std::optional<std::uint16_t> headIconId;

    std::uint16_t scaleX = 128;
    std::uint16_t scaleY = 128;

    std::int8_t ambient = 0;
    std::int8_t contrast = 0;
    std::uint16_t rotationSpeed = 32;

    std::array<std::optional<std::uint16_t>, 3>
        additionalModelIds;

    std::optional<std::uint16_t> morphVarbitId;
    std::optional<std::uint16_t> morphVarpId;
    std::vector<std::optional<std::uint16_t>> morphIds;
};

}
