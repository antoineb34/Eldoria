#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace eld::definition {

struct LocationModel {
    std::uint16_t id = 0;
    std::optional<std::uint8_t> type;
};

struct LocationRecolor {
    std::uint16_t source = 0;
    std::uint16_t destination = 0;
};

struct LocationDefinition {
    std::uint16_t id = 0;

    std::string name = "null";
    std::string description;

    std::vector<LocationModel> models;
    std::vector<LocationRecolor> recolors;

    std::uint8_t width = 1;
    std::uint8_t length = 1;

    bool solid = true;
    bool impenetrable = true;
    bool contouredGround = false;
    bool nonFlatShading = false;
    bool modelClipped = false;
    bool rotated = false;
    bool castsShadow = true;
    bool obstructsGround = false;
    bool hollow = false;

    std::optional<std::uint8_t> interactionType;
    std::optional<std::uint16_t> animationId;

    std::uint8_t decorDisplacement = 16;
    std::int8_t ambient = 0;
    std::int8_t contrast = 0;

    std::array<std::string, 5> actions;

    std::optional<std::uint16_t> mapFunctionId;
    std::optional<std::uint16_t> mapSceneId;

    std::uint16_t scaleX = 128;
    std::uint16_t scaleY = 128;
    std::uint16_t scaleZ = 128;

    std::int16_t offsetX = 0;
    std::int16_t offsetY = 0;
    std::int16_t offsetZ = 0;

    std::uint8_t surroundings = 0;
    std::optional<std::uint8_t> supportItems;

    std::optional<std::uint16_t> morphVarbitId;
    std::optional<std::uint16_t> morphVarpId;
    std::vector<std::optional<std::uint16_t>> morphIds;
};

}
