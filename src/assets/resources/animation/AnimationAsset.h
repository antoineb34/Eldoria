#pragma once

#include <cstdint>
#include <vector>

namespace eld::animation {

enum class TransformType : std::uint8_t {
    Pivot = 0,
    Translate = 1,
    Rotate = 2,
    Scale = 3,
    Unknown4 = 4,
    Alpha = 5
};

struct SkeletonSlot {
    std::uint8_t type = 0;
    std::vector<std::uint8_t> groups;
};

struct Skeleton {
    std::vector<SkeletonSlot> slots;
};

struct FrameTransform {
    std::uint16_t slot = 0;
    std::uint8_t flags = 0;

    int x = 0;
    int y = 0;
    int z = 0;
};

struct AnimationFrame {
    std::uint16_t id = 0;
    std::uint8_t slotCount = 0;
    std::uint8_t delay = 0;

    std::vector<FrameTransform> transforms;
};

struct AnimationAsset {
    Skeleton skeleton;
    std::vector<AnimationFrame> frames;
};

}
