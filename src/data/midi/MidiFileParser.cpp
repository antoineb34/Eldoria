#include "MidiFileParser.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace eld::midi {
namespace {

bool matches(
    std::span<const std::uint8_t> bytes,
    std::size_t offset,
    std::string_view text
) {
    if (offset + text.size() > bytes.size()) {
        return false;
    }

    for (std::size_t index = 0; index < text.size(); ++index) {
        if (
            bytes[offset + index] !=
            static_cast<std::uint8_t>(text[index])
        ) {
            return false;
        }
    }

    return true;
}

std::optional<std::uint16_t> readBe16(
    std::span<const std::uint8_t> bytes,
    std::size_t offset
) {
    if (offset + 2 > bytes.size()) {
        return std::nullopt;
    }

    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8) |
        static_cast<std::uint16_t>(bytes[offset + 1])
    );
}

std::optional<std::uint32_t> readBe32(
    std::span<const std::uint8_t> bytes,
    std::size_t offset
) {
    if (offset + 4 > bytes.size()) {
        return std::nullopt;
    }

    return
        (static_cast<std::uint32_t>(bytes[offset]) << 24) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 16) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 8) |
        static_cast<std::uint32_t>(bytes[offset + 3]);
}

std::optional<std::uint32_t> readLe32(
    std::span<const std::uint8_t> bytes,
    std::size_t offset
) {
    if (offset + 4 > bytes.size()) {
        return std::nullopt;
    }

    return
        static_cast<std::uint32_t>(bytes[offset]) |
        (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
        (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
        (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::optional<std::span<const std::uint8_t>> extractRiffMidiData(
    std::span<const std::uint8_t> bytes
) {
    if (
        bytes.size() < 12 ||
        !matches(bytes, 0, "RIFF") ||
        !matches(bytes, 8, "RMID")
    ) {
        return std::nullopt;
    }

    const std::optional<std::uint32_t> riffSize =
        readLe32(bytes, 4);

    if (!riffSize.has_value()) {
        return std::nullopt;
    }

    const std::uint64_t declaredEnd64 =
        static_cast<std::uint64_t>(*riffSize) + 8u;

    if (
        declaredEnd64 > bytes.size() ||
        declaredEnd64 > std::numeric_limits<std::size_t>::max()
    ) {
        return std::nullopt;
    }

    const std::size_t declaredEnd =
        static_cast<std::size_t>(declaredEnd64);

    std::size_t cursor = 12;

    while (cursor < declaredEnd) {
        if (cursor + 8 > declaredEnd) {
            return std::nullopt;
        }

        const std::optional<std::uint32_t> chunkSize =
            readLe32(bytes, cursor + 4);

        if (!chunkSize.has_value()) {
            return std::nullopt;
        }

        const std::size_t dataOffset = cursor + 8;
        const std::uint64_t dataEnd64 =
            static_cast<std::uint64_t>(dataOffset) +
            static_cast<std::uint64_t>(*chunkSize);

        if (dataEnd64 > declaredEnd) {
            return std::nullopt;
        }

        const std::size_t dataEnd =
            static_cast<std::size_t>(dataEnd64);

        if (matches(bytes, cursor, "data")) {
            return bytes.subspan(
                dataOffset,
                static_cast<std::size_t>(*chunkSize)
            );
        }

        const std::uint64_t next64 =
            dataEnd64 + static_cast<std::uint64_t>(*chunkSize & 1u);

        if (next64 > declaredEnd) {
            return std::nullopt;
        }

        cursor = static_cast<std::size_t>(next64);
    }

    return std::nullopt;
}

std::optional<MidiFileData> parseStandardMidi(
    std::span<const std::uint8_t> bytes,
    MidiContainer sourceContainer
) {
    if (bytes.size() < 14 || !matches(bytes, 0, "MThd")) {
        return std::nullopt;
    }

    const std::optional<std::uint32_t> headerSize =
        readBe32(bytes, 4);

    if (!headerSize.has_value() || *headerSize < 6) {
        return std::nullopt;
    }

    const std::uint64_t headerEnd64 =
        8u + static_cast<std::uint64_t>(*headerSize);

    if (headerEnd64 > bytes.size()) {
        return std::nullopt;
    }

    const std::optional<std::uint16_t> format =
        readBe16(bytes, 8);
    const std::optional<std::uint16_t> trackCount =
        readBe16(bytes, 10);
    const std::optional<std::uint16_t> division =
        readBe16(bytes, 12);

    if (
        !format.has_value() ||
        !trackCount.has_value() ||
        !division.has_value() ||
        *format > 2 ||
        *trackCount == 0 ||
        (*format == 0 && *trackCount != 1) ||
        *division == 0
    ) {
        return std::nullopt;
    }

    MidiFileData result;
    result.sourceContainer = sourceContainer;
    result.header = MidiHeader{
        .format = *format,
        .trackCount = *trackCount,
        .division = *division
    };
    result.tracks.reserve(*trackCount);

    std::size_t cursor =
        static_cast<std::size_t>(headerEnd64);

    for (
        std::uint16_t trackIndex = 0;
        trackIndex < *trackCount;
        ++trackIndex
    ) {
        if (
            cursor + 8 > bytes.size() ||
            !matches(bytes, cursor, "MTrk")
        ) {
            return std::nullopt;
        }

        const std::optional<std::uint32_t> trackSize =
            readBe32(bytes, cursor + 4);

        if (!trackSize.has_value()) {
            return std::nullopt;
        }

        const std::size_t dataOffset = cursor + 8;
        const std::uint64_t trackEnd64 =
            static_cast<std::uint64_t>(dataOffset) +
            static_cast<std::uint64_t>(*trackSize);

        if (trackEnd64 > bytes.size()) {
            return std::nullopt;
        }

        result.tracks.push_back(
            MidiTrackInfo{
                .chunkOffset = cursor,
                .dataOffset = dataOffset,
                .dataSize = *trackSize
            }
        );

        cursor = static_cast<std::size_t>(trackEnd64);
    }

    result.bytes.assign(
        bytes.begin(),
        bytes.end()
    );

    return result;
}

} // namespace

std::optional<MidiFileData> MidiFileParser::parse(
    std::span<const std::uint8_t> bytes
) const {
    if (matches(bytes, 0, "MThd")) {
        return parseStandardMidi(
            bytes,
            MidiContainer::StandardMidi
        );
    }

    const std::optional<std::span<const std::uint8_t>> riffData =
        extractRiffMidiData(bytes);

    if (!riffData.has_value()) {
        return std::nullopt;
    }

    return parseStandardMidi(
        *riffData,
        MidiContainer::RiffMidi
    );
}

}
