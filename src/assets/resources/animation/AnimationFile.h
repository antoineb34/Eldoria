#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace eld::animation {

struct AnimationFooter {
    std::uint16_t frameHeaderBytes = 0;
    std::uint16_t flagBytes = 0;
    std::uint16_t valueBytes = 0;
    std::uint16_t delayBytes = 0;
};

struct AnimationLayout {
    std::size_t frameCountOffset = 0;
    std::size_t frameHeaderOffset = 2;
    std::size_t flagOffset = 0;
    std::size_t valueOffset = 0;
    std::size_t delayOffset = 0;
    std::size_t skeletonOffset = 0;
    std::size_t footerOffset = 0;
};

struct AnimationFile {
    std::vector<std::uint8_t> bytes;
    std::uint16_t frameCount = 0;
    AnimationFooter footer;
    AnimationLayout layout;
};

}
