#pragma once

#include <cstdint>
#include <optional>
#include <vector>

namespace eld::definition {

struct SequenceFrame {
    std::uint16_t primaryFrameId = 0;
    std::optional<std::uint16_t> secondaryFrameId;
    std::uint16_t duration = 0;
};

struct SequenceDefinition {
    std::uint16_t id = 0;

    std::vector<SequenceFrame> frames;
    std::optional<std::uint16_t> frameStep;
    std::vector<std::uint8_t> interleaveOrder;

    bool stretches = false;

    std::uint8_t priority = 5;
    std::optional<std::uint16_t> shieldItemId;
    std::optional<std::uint16_t> weaponItemId;

    std::uint8_t maximumLoops = 99;

    std::optional<std::uint8_t> animatingPrecedence;
    std::optional<std::uint8_t> walkingPrecedence;

    std::uint8_t replayMode = 2;
    std::optional<std::uint32_t> packedData;
};

}
