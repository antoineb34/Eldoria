#include "decoders/AnimationDecoder.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>

#include "binary/ByteReader.h"

namespace eld::animation {

Animation AnimationDecoder::decode(
    std::span<const std::uint8_t> payload
) const {
    constexpr std::size_t FooterSize = 8;
    constexpr std::size_t FrameCountSize = 2;
    constexpr std::size_t FrameHeaderSize = 3;

    if (
        payload.size() <
        FrameCountSize + FooterSize
    ) {
        throw std::runtime_error(
            "Animation payload is too small"
        );
    }

    eld::binary::ByteReader header(
        payload.first(FrameCountSize)
    );

    const auto frameCount =
        header.readU16();

    eld::binary::ByteReader footer(
        payload.last(FooterSize)
    );

    const auto frameHeaderBytes =
        footer.readU16();

    const auto flagBytes =
        footer.readU16();

    const auto valueBytes =
        footer.readU16();

    const auto delayBytes =
        footer.readU16();

    if (
        frameHeaderBytes !=
        static_cast<std::size_t>(
            frameCount
        ) *
        FrameHeaderSize
    ) {
        throw std::runtime_error(
            "Invalid animation frame-header length"
        );
    }

    if (
        delayBytes !=
        frameCount
    ) {
        throw std::runtime_error(
            "Invalid animation delay length"
        );
    }


    // Body streams

    const auto body =
        payload.first(
            payload.size() -
            FooterSize
        );

    std::size_t offset =
        FrameCountSize;

    auto section = [&](std::size_t size) {
        if (
            offset > body.size() ||
            size > body.size() - offset
        ) {
            throw std::runtime_error(
                "Animation section exceeds body"
            );
        }

        eld::binary::ByteReader reader(
            body.subspan(
                offset,
                size
            )
        );

        offset += size;
        return reader;
    };

    auto frameHeaders =
        section(frameHeaderBytes);

    auto flags =
        section(flagBytes);

    auto values =
        section(valueBytes);

    auto delays =
        section(delayBytes);

    if (
        offset >=
        body.size()
    ) {
        throw std::runtime_error(
            "Invalid animation layout"
        );
    }

    auto skeletonData =
        section(
            body.size() -
            offset
        );


    // Animation

    Animation animation;


    // Skeleton

    const auto skeletonSlotCount =
        skeletonData.readU8();

    animation.skeleton.resize(
        skeletonSlotCount
    );

    for (
        SkeletonSlot& slot :
        animation.skeleton
    ) {
        slot.type =
            static_cast<TransformType>(
                skeletonData.readU8()
            );
    }

    for (
        SkeletonSlot& slot :
        animation.skeleton
    ) {
        const auto groupCount =
            skeletonData.readU8();

        slot.groups.reserve(
            groupCount
        );

        for (
            std::size_t group = 0;
            group < groupCount;
            ++group
        ) {
            slot.groups.push_back(
                skeletonData.readU8()
            );
        }
    }

    if (!skeletonData.atEnd()) {
        throw std::runtime_error(
            "Animation skeleton did not consume its section exactly"
        );
    }


    // Frames

    animation.frames.reserve(
        frameCount
    );

    for (
        std::size_t frameIndex = 0;
        frameIndex < frameCount;
        ++frameIndex
    ) {
        AnimationFrame frame;

        frame.id =
            frameHeaders.readU16();

        const auto slotCount =
            frameHeaders.readU8();

        if (
            slotCount >
            animation.skeleton.size()
        ) {
            throw std::runtime_error(
                "Animation frame references more slots than its skeleton owns"
            );
        }

        for (
            std::size_t slotIndex = 0;
            slotIndex < slotCount;
            ++slotIndex
        ) {
            const auto sourceFlags =
                flags.readU8();

            if (sourceFlags == 0) {
                continue;
            }

            const TransformType transformType =
                animation.skeleton[
                    slotIndex
                ].type;

            const int defaultValue =
                transformType ==
                    TransformType::Scale
                    ? 128
                    : 0;

            FrameTransform transform;

            transform.slot =
                static_cast<std::uint16_t>(
                    slotIndex
                );

            transform.x = defaultValue;
            transform.y = defaultValue;
            transform.z = defaultValue;

            if (sourceFlags & 1) {
                transform.x =
                    values.readSignedSmart();
            }

            if (sourceFlags & 2) {
                transform.y =
                    values.readSignedSmart();
            }

            if (sourceFlags & 4) {
                transform.z =
                    values.readSignedSmart();
            }

            frame.transforms.push_back(
                transform
            );
        }

        frame.delay =
            delays.readU8();

        animation.frames.push_back(
            std::move(frame)
        );
    }

    if (
        !frameHeaders.atEnd() ||
        !flags.atEnd() ||
        !values.atEnd() ||
        !delays.atEnd()
    ) {
        throw std::runtime_error(
            "Animation streams were not consumed exactly"
        );
    }

    return animation;
}

}
