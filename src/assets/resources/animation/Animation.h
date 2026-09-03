#pragma once

#include <cstdint>
#include <span>
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
    TransformType type = TransformType::Pivot;
    std::vector<std::uint8_t> groups;
};

struct FrameTransform {
    std::uint16_t slot = 0;

    int x = 0;
    int y = 0;
    int z = 0;
};

struct AnimationFrame {
    std::uint16_t id = 0;
    std::uint8_t delay = 0;

    std::vector<FrameTransform> transforms;
};

struct Animation {
    std::uint16_t id = 0;

    std::vector<SkeletonSlot> skeleton;
    std::vector<AnimationFrame> frames;
};

struct AnimationFrameView {
    const AnimationFrame& frame;
    std::span<const SkeletonSlot> skeleton;
};

}
