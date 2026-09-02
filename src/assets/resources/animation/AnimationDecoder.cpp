#include "AnimationDecoder.h"

#include "AnimationFileParser.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace eld::animation {

namespace {

std::uint16_t readU16(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::size_t end
) {
    if (
        offset + 2 >
        end
    ) {
        throw std::runtime_error(
            "Animation u16 read crossed section boundary"
        );
    }

    const std::uint16_t value =
        static_cast<std::uint16_t>(
            (
                static_cast<std::uint16_t>(
                    bytes[offset]
                ) << 8
            ) |
            static_cast<std::uint16_t>(
                bytes[offset + 1]
            )
        );

    offset += 2;

    return value;
}

std::uint8_t readU8(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::size_t end
) {
    if (
        offset >=
        end
    ) {
        throw std::runtime_error(
            "Animation u8 read crossed section boundary"
        );
    }

    return bytes[offset++];
}

int readSignedSmart(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::size_t end
) {
    if (
        offset >=
        end
    ) {
        throw std::runtime_error(
            "Animation signed-smart section ended early"
        );
    }

    const std::uint8_t peek =
        bytes[offset];

    if (
        peek <
        128
    ) {
        ++offset;

        return
            static_cast<int>(
                peek
            ) -
            64;
    }

    return
        static_cast<int>(
            readU16(
                bytes,
                offset,
                end
            )
        ) -
        49152;
}

Skeleton decodeSkeleton(
    const AnimationFile& file
) {
    const std::vector<std::uint8_t>& bytes =
        file.bytes;

    std::size_t offset =
        file.layout.skeletonOffset;

    const std::size_t end =
        file.layout.footerOffset;

    const std::uint8_t slotCount =
        readU8(
            bytes,
            offset,
            end
        );

    Skeleton skeleton;

    skeleton.slots.resize(
        slotCount
    );

    for (
        std::size_t slot = 0;
        slot < slotCount;
        ++slot
    ) {
        skeleton.slots[slot].type =
            readU8(
                bytes,
                offset,
                end
            );
    }

    for (
        std::size_t slot = 0;
        slot < slotCount;
        ++slot
    ) {
        const std::uint8_t groupCount =
            readU8(
                bytes,
                offset,
                end
            );

        std::vector<std::uint8_t>& groups =
            skeleton.slots[slot].groups;

        groups.reserve(
            groupCount
        );

        for (
            std::size_t group = 0;
            group < groupCount;
            ++group
        ) {
            groups.push_back(
                readU8(
                    bytes,
                    offset,
                    end
                )
            );
        }
    }

    if (
        offset !=
        end
    ) {
        throw std::runtime_error(
            "Animation skeleton did not consume its section exactly"
        );
    }

    return skeleton;
}

struct FrameHeader {
    std::uint16_t id = 0;
    std::uint8_t slotCount = 0;
};

std::vector<FrameHeader> decodeFrameHeaders(
    const AnimationFile& file
) {
    const std::vector<std::uint8_t>& bytes =
        file.bytes;

    std::size_t offset =
        file.layout.frameHeaderOffset;

    const std::size_t end =
        file.layout.flagOffset;

    std::vector<FrameHeader> headers;

    headers.reserve(
        file.frameCount
    );

    for (
        std::size_t frame = 0;
        frame < file.frameCount;
        ++frame
    ) {
        FrameHeader header;

        header.id =
            readU16(
                bytes,
                offset,
                end
            );

        header.slotCount =
            readU8(
                bytes,
                offset,
                end
            );

        headers.push_back(
            header
        );
    }

    if (
        offset !=
        end
    ) {
        throw std::runtime_error(
            "Animation frame-header section was not consumed exactly"
        );
    }

    return headers;
}

}

AnimationAsset AnimationDecoder::decodeAsset(
    const AnimationFile& file
) const {
    AnimationAsset asset;

    asset.skeleton =
        decodeSkeleton(
            file
        );

    const std::vector<FrameHeader> headers =
        decodeFrameHeaders(
            file
        );

    const std::vector<std::uint8_t>& bytes =
        file.bytes;

    std::size_t flagOffset =
        file.layout.flagOffset;

    const std::size_t flagEnd =
        file.layout.valueOffset;

    std::size_t valueOffset =
        file.layout.valueOffset;

    const std::size_t valueEnd =
        file.layout.delayOffset;

    std::size_t delayOffset =
        file.layout.delayOffset;

    const std::size_t delayEnd =
        file.layout.skeletonOffset;

    asset.frames.reserve(
        headers.size()
    );

    for (
        const FrameHeader& header :
        headers
    ) {
        if (
            header.slotCount >
            asset.skeleton.slots.size()
        ) {
            throw std::runtime_error(
                "Animation frame references more slots than its skeleton owns"
            );
        }

        AnimationFrame frame;

        frame.id =
            header.id;

        frame.slotCount =
            header.slotCount;

        std::vector<std::uint8_t> flags;

        flags.reserve(
            header.slotCount
        );

        for (
            std::size_t slot = 0;
            slot < header.slotCount;
            ++slot
        ) {
            flags.push_back(
                readU8(
                    bytes,
                    flagOffset,
                    flagEnd
                )
            );
        }

        for (
            std::size_t slot = 0;
            slot < flags.size();
            ++slot
        ) {
            const std::uint8_t sourceFlags =
                flags[slot];

            if (
                sourceFlags ==
                0
            ) {
                continue;
            }

            const std::uint8_t transformType =
                asset.skeleton.slots[slot].type;

            const int defaultValue =
                transformType ==
                    static_cast<std::uint8_t>(
                        TransformType::Scale
                    )
                    ? 128
                    : 0;

            FrameTransform transform;

            transform.slot =
                static_cast<std::uint16_t>(
                    slot
                );

            transform.flags =
                sourceFlags;

            transform.x =
                defaultValue;

            transform.y =
                defaultValue;

            transform.z =
                defaultValue;

            if (
                sourceFlags &
                0x1
            ) {
                transform.x =
                    readSignedSmart(
                        bytes,
                        valueOffset,
                        valueEnd
                    );
            }

            if (
                sourceFlags &
                0x2
            ) {
                transform.y =
                    readSignedSmart(
                        bytes,
                        valueOffset,
                        valueEnd
                    );
            }

            if (
                sourceFlags &
                0x4
            ) {
                transform.z =
                    readSignedSmart(
                        bytes,
                        valueOffset,
                        valueEnd
                    );
            }

            frame.transforms.push_back(
                transform
            );
        }

        frame.delay =
            readU8(
                bytes,
                delayOffset,
                delayEnd
            );

        asset.frames.push_back(
            std::move(
                frame
            )
        );
    }

    if (
        flagOffset !=
        flagEnd
    ) {
        throw std::runtime_error(
            "Animation flag section was not consumed exactly"
        );
    }

    if (
        valueOffset !=
        valueEnd
    ) {
        throw std::runtime_error(
            "Animation transform-value section was not consumed exactly"
        );
    }

    if (
        delayOffset !=
        delayEnd
    ) {
        throw std::runtime_error(
            "Animation delay section was not consumed exactly"
        );
    }

    return asset;
}

Animation AnimationDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    AnimationFileParser parser;

    std::vector<std::uint8_t> bytes(
        payload.begin(),
        payload.end()
    );

    std::optional<AnimationFile> file =
        parser.parse(bytes);

    if (!file.has_value()) {
        throw std::runtime_error(
            "Invalid animation payload"
        );
    }

    AnimationAsset asset =
        decodeAsset(*file);

    return Animation{
        .bytes = std::move(file->bytes),
        .asset = std::move(asset)
    };
}

}
