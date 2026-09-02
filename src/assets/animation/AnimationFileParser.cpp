#include "AnimationFileParser.h"

#include <cstddef>
#include <cstdint>
#include <limits>

namespace eld::animation {

namespace {

constexpr std::size_t FooterBytes = 8;
constexpr std::size_t FrameCountBytes = 2;
constexpr std::size_t FrameHeaderBytesPerFrame = 3;

std::uint16_t readU16(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset
) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes.at(offset)) << 8) |
        static_cast<std::uint16_t>(bytes.at(offset + 1))
    );
}

bool addChecked(
    std::size_t& value,
    std::size_t amount
) {
    if (
        amount >
        std::numeric_limits<std::size_t>::max() - value
    ) {
        return false;
    }

    value += amount;
    return true;
}

}

std::optional<AnimationFile>
AnimationFileParser::parse(
    const std::vector<std::uint8_t>& bytes
) const {
    if (
        bytes.size() <
        FrameCountBytes + FooterBytes
    ) {
        return std::nullopt;
    }

    AnimationFile file;
    file.bytes = bytes;

    file.frameCount =
        readU16(
            file.bytes,
            0
        );

    file.layout.footerOffset =
        file.bytes.size() -
        FooterBytes;

    file.footer.frameHeaderBytes =
        readU16(
            file.bytes,
            file.layout.footerOffset
        );

    file.footer.flagBytes =
        readU16(
            file.bytes,
            file.layout.footerOffset + 2
        );

    file.footer.valueBytes =
        readU16(
            file.bytes,
            file.layout.footerOffset + 4
        );

    file.footer.delayBytes =
        readU16(
            file.bytes,
            file.layout.footerOffset + 6
        );

    const std::size_t expectedFrameHeaderBytes =
        static_cast<std::size_t>(
            file.frameCount
        ) *
        FrameHeaderBytesPerFrame;

    if (
        file.footer.frameHeaderBytes !=
        expectedFrameHeaderBytes
    ) {
        return std::nullopt;
    }

    if (
        file.footer.delayBytes !=
        static_cast<std::size_t>(
            file.frameCount
        )
    ) {
        return std::nullopt;
    }

    std::size_t offset =
        file.layout.frameHeaderOffset;

    if (
        !addChecked(
            offset,
            file.footer.frameHeaderBytes
        )
    ) {
        return std::nullopt;
    }

    file.layout.flagOffset =
        offset;

    if (
        !addChecked(
            offset,
            file.footer.flagBytes
        )
    ) {
        return std::nullopt;
    }

    file.layout.valueOffset =
        offset;

    if (
        !addChecked(
            offset,
            file.footer.valueBytes
        )
    ) {
        return std::nullopt;
    }

    file.layout.delayOffset =
        offset;

    if (
        !addChecked(
            offset,
            file.footer.delayBytes
        )
    ) {
        return std::nullopt;
    }

    file.layout.skeletonOffset =
        offset;

    if (
        file.layout.skeletonOffset >=
        file.layout.footerOffset
    ) {
        return std::nullopt;
    }

    return file;
}

}
