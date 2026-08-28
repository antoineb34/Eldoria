#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "archive/Archive.h"
#include "archive/ArchiveParser.h"

#include "binary/Compression.h"

#include "cache/Cache.h"
#include "cache/File.h"
#include "cache/Index.h"
#include "cache/Store.h"

namespace {

constexpr std::uint16_t VersionListArchiveId = 5;

constexpr std::size_t SummaryHeadBytes = 12;
constexpr std::size_t SummaryTailBytes = 8;
constexpr std::size_t DefaultDetailBytes = 256;

struct AnimationVersionList {
    // anim_index is a big-endian u16 table.  Its exact gameplay/
    // on-demand meaning is deliberately left unresolved here.
    // It is NOT a frame-id -> Index 2 archive-id mapping.
    std::vector<std::uint16_t> animIndexEntries;

    std::vector<std::uint32_t> crcs;
    std::vector<std::uint16_t> versions;

    std::string warning;

    bool loaded() const {
        return
            !animIndexEntries.empty() ||
            !crcs.empty() ||
            !versions.empty();
    }
};

std::uint16_t readU16BigEndian(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset
) {
    if (offset + 2 > bytes.size()) {
        throw std::out_of_range(
            "u16 read exceeds buffer"
        );
    }

    return
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
}

std::uint32_t readU32BigEndian(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset
) {
    if (offset + 4 > bytes.size()) {
        throw std::out_of_range(
            "u32 read exceeds buffer"
        );
    }

    return
        (
            static_cast<std::uint32_t>(
                bytes[offset]
            ) << 24
        ) |
        (
            static_cast<std::uint32_t>(
                bytes[offset + 1]
            ) << 16
        ) |
        (
            static_cast<std::uint32_t>(
                bytes[offset + 2]
            ) << 8
        ) |
        static_cast<std::uint32_t>(
            bytes[offset + 3]
        );
}

std::vector<std::uint16_t> decodeU16Table(
    const std::vector<std::uint8_t>& bytes
) {
    std::vector<std::uint16_t> values;

    values.reserve(
        bytes.size() / 2
    );

    for (
        std::size_t offset = 0;
        offset + 2 <= bytes.size();
        offset += 2
    ) {
        values.push_back(
            readU16BigEndian(
                bytes,
                offset
            )
        );
    }

    return values;
}

std::vector<std::uint32_t> decodeU32Table(
    const std::vector<std::uint8_t>& bytes
) {
    std::vector<std::uint32_t> values;

    values.reserve(
        bytes.size() / 4
    );

    for (
        std::size_t offset = 0;
        offset + 4 <= bytes.size();
        offset += 4
    ) {
        values.push_back(
            readU32BigEndian(
                bytes,
                offset
            )
        );
    }

    return values;
}

AnimationVersionList loadAnimationVersionList(
    const eld::cache::Store& configStore
) {
    AnimationVersionList result;

    try {
        const eld::cache::File cacheFile =
            configStore.get(
                VersionListArchiveId
            );

        eld::archive::ArchiveParser parser;

        const std::optional<eld::archive::Archive> archive =
            parser.parse(
                cacheFile.getBytes()
            );

        if (!archive.has_value()) {
            result.warning =
                "config archive 5 did not parse as an archive";

            return result;
        }

        try {
            result.animIndexEntries =
                decodeU16Table(
                    archive->get(
                        "anim_index"
                    ).payload
                );
        }
        catch (const std::exception&) {
        }

        try {
            result.crcs =
                decodeU32Table(
                    archive->get(
                        "anim_crc"
                    ).payload
                );
        }
        catch (const std::exception&) {
        }

        try {
            result.versions =
                decodeU16Table(
                    archive->get(
                        "anim_version"
                    ).payload
                );
        }
        catch (const std::exception&) {
        }

        if (!result.loaded()) {
            result.warning =
                "archive 5 parsed, but animation metadata files were not found";
        }
    }
    catch (const std::exception& exception) {
        result.warning =
            std::string(
                "could not load config archive 5: "
            ) +
            exception.what();
    }

    return result;
}

const char* compressionName(
    eld::binary::CompressionType type
) {
    switch (type) {
        case eld::binary::CompressionType::None:
            return "none";

        case eld::binary::CompressionType::Gzip:
            return "gzip";

        case eld::binary::CompressionType::Bzip2:
            return "bzip2";

        default:
            return "unknown";
    }
}

std::string hexSlice(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t amount
) {
    if (
        bytes.empty() ||
        offset >= bytes.size()
    ) {
        return "-";
    }

    const std::size_t end =
        std::min(
            bytes.size(),
            offset + amount
        );

    std::ostringstream stream;

    stream
        << std::hex
        << std::setfill('0');

    for (
        std::size_t index = offset;
        index < end;
        ++index
    ) {
        if (index != offset) {
            stream << ' ';
        }

        stream
            << std::setw(2)
            << static_cast<unsigned int>(
                bytes[index]
            );
    }

    return stream.str();
}

std::string tailHex(
    const std::vector<std::uint8_t>& bytes,
    std::size_t amount
) {
    if (bytes.empty()) {
        return "-";
    }

    const std::size_t offset =
        bytes.size() > amount
            ? bytes.size() - amount
            : 0;

    return hexSlice(
        bytes,
        offset,
        amount
    );
}

std::string asciiSlice(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t amount
) {
    if (
        bytes.empty() ||
        offset >= bytes.size()
    ) {
        return {};
    }

    const std::size_t end =
        std::min(
            bytes.size(),
            offset + amount
        );

    std::string text;

    text.reserve(
        end - offset
    );

    for (
        std::size_t index = offset;
        index < end;
        ++index
    ) {
        const unsigned char value =
            bytes[index];

        text.push_back(
            std::isprint(value)
                ? static_cast<char>(value)
                : '.'
        );
    }

    return text;
}

void printHexDump(
    const std::vector<std::uint8_t>& bytes,
    std::size_t offset,
    std::size_t amount
) {
    if (
        bytes.empty() ||
        offset >= bytes.size()
    ) {
        std::cout << "  (empty)\n";
        return;
    }

    const std::size_t end =
        std::min(
            bytes.size(),
            offset + amount
        );

    for (
        std::size_t row = offset;
        row < end;
        row += 16
    ) {
        std::cout
            << "  "
            << std::hex
            << std::setw(6)
            << std::setfill('0')
            << row
            << "  ";

        for (
            std::size_t column = 0;
            column < 16;
            ++column
        ) {
            const std::size_t index =
                row + column;

            if (index < end) {
                std::cout
                    << std::setw(2)
                    << static_cast<unsigned int>(
                        bytes[index]
                    )
                    << ' ';
            }
            else {
                std::cout << "   ";
            }
        }

        std::cout << " ";

        const std::size_t rowEnd =
            std::min(
                end,
                row + 16
            );

        for (
            std::size_t index = row;
            index < rowEnd;
            ++index
        ) {
            const unsigned char value =
                bytes[index];

            std::cout
                << (
                    std::isprint(value)
                        ? static_cast<char>(value)
                        : '.'
                );
        }

        std::cout << '\n';
    }

    std::cout << std::dec;
}

void printTailHexDump(
    const std::vector<std::uint8_t>& bytes,
    std::size_t amount
) {
    if (bytes.empty()) {
        std::cout << "  (empty)\n";
        return;
    }

    const std::size_t offset =
        bytes.size() > amount
            ? bytes.size() - amount
            : 0;

    printHexDump(
        bytes,
        offset,
        bytes.size() - offset
    );
}

template <typename T>
std::optional<T> valueAt(
    const std::vector<T>& values,
    std::uint16_t id
) {
    const std::size_t index =
        static_cast<std::size_t>(
            id
        );

    if (index >= values.size()) {
        return std::nullopt;
    }

    return values[index];
}

void printVersionMetadata(
    const AnimationVersionList& versionList,
    std::uint16_t id,
    std::string_view prefix = ""
) {
    if (
        const std::optional<std::uint16_t> version =
            valueAt(
                versionList.versions,
                id
            )
    ) {
        std::cout
            << prefix
            << "version="
            << *version
            << ' ';
    }

    if (
        const std::optional<std::uint32_t> crc =
            valueAt(
                versionList.crcs,
                id
            )
    ) {
        std::cout
            << prefix
            << "crc=0x"
            << std::hex
            << std::setw(8)
            << std::setfill('0')
            << *crc
            << std::dec
            << ' ';
    }
}

std::uint16_t parseFileId(
    const char* value
) {
    const unsigned long parsed =
        std::stoul(
            value
        );

    if (
        parsed >
        std::numeric_limits<std::uint16_t>::max()
    ) {
        throw std::out_of_range(
            "animation file id is outside uint16 range"
        );
    }

    return
        static_cast<std::uint16_t>(
            parsed
        );
}

std::size_t parseAmount(
    const char* value
) {
    const unsigned long long parsed =
        std::stoull(
            value
        );

    if (
        parsed >
        std::numeric_limits<std::size_t>::max()
    ) {
        throw std::out_of_range(
            "dump byte count is too large"
        );
    }

    return
        static_cast<std::size_t>(
            parsed
        );
}

void printVersionListSummary(
    const AnimationVersionList& versionList
) {
    std::cout << "\nArchive 5 animation metadata\n";
    std::cout << "----------------------------\n";

    if (!versionList.warning.empty()) {
        std::cout
            << "warning: "
            << versionList.warning
            << '\n';
    }

    std::cout
        << "anim_index u16 entries:     "
        << versionList.animIndexEntries.size()
        << '\n'
        << "anim_crc entries:           "
        << versionList.crcs.size()
        << '\n'
        << "anim_version entries:       "
        << versionList.versions.size()
        << '\n';
}


struct AnimationLayout {
    std::uint16_t frameHeaderBytes = 0;
    std::uint16_t flagBytes = 0;
    std::uint16_t valueBytes = 0;
    std::uint16_t delayBytes = 0;

    std::size_t frameHeaderOffset = 0;
    std::size_t flagOffset = 0;
    std::size_t valueOffset = 0;
    std::size_t delayOffset = 0;
    std::size_t skeletonOffset = 0;
    std::size_t footerOffset = 0;
};

struct SkeletonSlotProbe {
    std::uint8_t transformType = 0;
    std::vector<std::uint8_t> groups;
};

struct SkeletonProbe {
    std::vector<SkeletonSlotProbe> slots;
};

struct FrameComponentProbe {
    std::size_t slot = 0;
    std::uint8_t flags = 0;

    std::optional<int> x;
    std::optional<int> y;
    std::optional<int> z;
};

struct FrameProbe {
    std::uint16_t id = 0;
    std::uint8_t slotCount = 0;
    std::uint8_t delay = 0;

    std::vector<std::uint8_t> flags;
    std::vector<FrameComponentProbe> components;
};

AnimationLayout readAnimationLayout(
    const std::vector<std::uint8_t>& bytes
) {
    constexpr std::size_t FooterBytes = 8;

    if (bytes.size() < FooterBytes + 2) {
        throw std::runtime_error(
            "animation payload is too small for its footer"
        );
    }

    AnimationLayout layout;

    layout.footerOffset =
        bytes.size() -
        FooterBytes;

    layout.frameHeaderBytes =
        readU16BigEndian(
            bytes,
            layout.footerOffset
        );

    layout.flagBytes =
        readU16BigEndian(
            bytes,
            layout.footerOffset + 2
        );

    layout.valueBytes =
        readU16BigEndian(
            bytes,
            layout.footerOffset + 4
        );

    layout.delayBytes =
        readU16BigEndian(
            bytes,
            layout.footerOffset + 6
        );

    layout.frameHeaderOffset = 0;

    // The first section also owns the leading u16 frame count.
    layout.flagOffset =
        2 +
        layout.frameHeaderBytes;

    layout.valueOffset =
        layout.flagOffset +
        layout.flagBytes;

    layout.delayOffset =
        layout.valueOffset +
        layout.valueBytes;

    layout.skeletonOffset =
        layout.delayOffset +
        layout.delayBytes;

    if (
        layout.skeletonOffset >
        layout.footerOffset
    ) {
        throw std::runtime_error(
            "animation footer section lengths exceed the payload"
        );
    }

    return layout;
}

int readSignedSmart(
    const std::vector<std::uint8_t>& bytes,
    std::size_t& offset,
    std::size_t end
) {
    if (offset >= end) {
        throw std::runtime_error(
            "animation transform value section ended early"
        );
    }

    const std::uint8_t peek =
        bytes[offset];

    if (peek < 128) {
        ++offset;

        return
            static_cast<int>(
                peek
            ) -
            64;
    }

    if (offset + 2 > end) {
        throw std::runtime_error(
            "animation signed-smart value is truncated"
        );
    }

    const int value =
        static_cast<int>(
            readU16BigEndian(
                bytes,
                offset
            )
        ) -
        49152;

    offset += 2;

    return value;
}

SkeletonProbe readSkeletonProbe(
    const std::vector<std::uint8_t>& bytes,
    const AnimationLayout& layout
) {
    std::size_t offset =
        layout.skeletonOffset;

    if (offset >= layout.footerOffset) {
        throw std::runtime_error(
            "animation has no skeleton block"
        );
    }

    const std::uint8_t slotCount =
        bytes[offset++];

    SkeletonProbe skeleton;

    skeleton.slots.resize(
        slotCount
    );

    if (
        offset +
        slotCount >
        layout.footerOffset
    ) {
        throw std::runtime_error(
            "skeleton transform-type table is truncated"
        );
    }

    for (
        std::size_t slot = 0;
        slot < slotCount;
        ++slot
    ) {
        skeleton.slots[slot].transformType =
            bytes[offset++];
    }

    for (
        std::size_t slot = 0;
        slot < slotCount;
        ++slot
    ) {
        if (offset >= layout.footerOffset) {
            throw std::runtime_error(
                "skeleton group table is truncated"
            );
        }

        const std::uint8_t groupCount =
            bytes[offset++];

        if (
            offset +
            groupCount >
            layout.footerOffset
        ) {
            throw std::runtime_error(
                "skeleton group list is truncated"
            );
        }

        skeleton.slots[slot].groups.assign(
            bytes.begin() +
                static_cast<std::ptrdiff_t>(
                    offset
                ),
            bytes.begin() +
                static_cast<std::ptrdiff_t>(
                    offset +
                    groupCount
                )
        );

        offset +=
            groupCount;
    }

    if (offset != layout.footerOffset) {
        throw std::runtime_error(
            "skeleton decoder did not consume the complete skeleton block"
        );
    }

    return skeleton;
}

std::vector<FrameProbe> readFrameProbes(
    const std::vector<std::uint8_t>& bytes,
    const AnimationLayout& layout
) {
    const std::uint16_t frameCount =
        readU16BigEndian(
            bytes,
            0
        );

    std::size_t headerOffset = 2;
    std::size_t flagOffset =
        layout.flagOffset;
    std::size_t valueOffset =
        layout.valueOffset;
    std::size_t delayOffset =
        layout.delayOffset;

    std::vector<FrameProbe> frames;

    frames.reserve(
        frameCount
    );

    for (
        std::size_t frameIndex = 0;
        frameIndex < frameCount;
        ++frameIndex
    ) {
        if (
            headerOffset + 3 >
            layout.flagOffset
        ) {
            throw std::runtime_error(
                "frame-header section ended early"
            );
        }

        if (
            delayOffset >=
            layout.skeletonOffset
        ) {
            throw std::runtime_error(
                "frame-delay section ended early"
            );
        }

        FrameProbe frame;

        frame.id =
            readU16BigEndian(
                bytes,
                headerOffset
            );

        headerOffset += 2;

        frame.slotCount =
            bytes[headerOffset++];

        frame.delay =
            bytes[delayOffset++];

        frame.flags.reserve(
            frame.slotCount
        );

        for (
            std::size_t slot = 0;
            slot < frame.slotCount;
            ++slot
        ) {
            if (
                flagOffset >=
                layout.valueOffset
            ) {
                throw std::runtime_error(
                    "frame-flag section ended early"
                );
            }

            const std::uint8_t flags =
                bytes[flagOffset++];

            frame.flags.push_back(
                flags
            );

            if (flags == 0) {
                continue;
            }

            FrameComponentProbe component;

            component.slot = slot;
            component.flags = flags;

            if ((flags & 0x1) != 0) {
                component.x =
                    readSignedSmart(
                        bytes,
                        valueOffset,
                        layout.delayOffset
                    );
            }

            if ((flags & 0x2) != 0) {
                component.y =
                    readSignedSmart(
                        bytes,
                        valueOffset,
                        layout.delayOffset
                    );
            }

            if ((flags & 0x4) != 0) {
                component.z =
                    readSignedSmart(
                        bytes,
                        valueOffset,
                        layout.delayOffset
                    );
            }

            frame.components.push_back(
                std::move(
                    component
                )
            );
        }

        frames.push_back(
            std::move(
                frame
            )
        );
    }

    if (
        headerOffset !=
        layout.flagOffset
    ) {
        throw std::runtime_error(
            "frame-header section length does not match the frame count"
        );
    }

    if (
        flagOffset !=
        layout.valueOffset
    ) {
        throw std::runtime_error(
            "frame-flag section was not consumed exactly"
        );
    }

    if (
        valueOffset !=
        layout.delayOffset
    ) {
        throw std::runtime_error(
            "frame-value section was not consumed exactly"
        );
    }

    if (
        delayOffset !=
        layout.skeletonOffset
    ) {
        throw std::runtime_error(
            "frame-delay section was not consumed exactly"
        );
    }

    return frames;
}

const char* transformTypeName(
    std::uint8_t type
) {
    switch (type) {
        case 0:
            return "pivot";

        case 1:
            return "translate";

        case 2:
            return "rotate";

        case 3:
            return "scale";

        case 5:
            return "alpha";

        default:
            return "unknown";
    }
}

std::string optionalComponent(
    const std::optional<int>& value
) {
    if (!value.has_value()) {
        return "-";
    }

    return
        std::to_string(
            *value
        );
}

void printAnimationStructure(
    const std::vector<std::uint8_t>& bytes
) {
    const AnimationLayout layout =
        readAnimationLayout(
            bytes
        );

    const std::vector<FrameProbe> frames =
        readFrameProbes(
            bytes,
            layout
        );

    const SkeletonProbe skeleton =
        readSkeletonProbe(
            bytes,
            layout
        );

    const std::size_t skeletonBytes =
        layout.footerOffset -
        layout.skeletonOffset;

    std::cout
        << "\nStructural decode\n"
        << "------------------------------\n"
        << "frame count:          "
        << frames.size()
        << '\n'
        << "frame-header section: offset 0, bytes "
        << (layout.frameHeaderBytes + 2)
        << " (2-byte count + "
        << layout.frameHeaderBytes
        << " frame-header bytes)\n"
        << "flag section:         offset "
        << layout.flagOffset
        << ", bytes "
        << layout.flagBytes
        << '\n'
        << "value section:        offset "
        << layout.valueOffset
        << ", bytes "
        << layout.valueBytes
        << '\n'
        << "delay section:        offset "
        << layout.delayOffset
        << ", bytes "
        << layout.delayBytes
        << '\n'
        << "skeleton section:     offset "
        << layout.skeletonOffset
        << ", bytes "
        << skeletonBytes
        << '\n'
        << "footer:               offset "
        << layout.footerOffset
        << ", bytes 8\n";

    std::cout
        << "\nSkeleton\n"
        << "------------------------------\n"
        << "slot count: "
        << skeleton.slots.size()
        << '\n';

    std::vector<std::size_t> transformTypeCounts(
        256,
        0
    );

    for (
        const SkeletonSlotProbe& slot :
        skeleton.slots
    ) {
        ++transformTypeCounts[
            slot.transformType
        ];
    }

    std::cout
        << "transform type histogram:";

    bool printedType = false;

    for (
        std::size_t type = 0;
        type < transformTypeCounts.size();
        ++type
    ) {
        if (transformTypeCounts[type] == 0) {
            continue;
        }

        std::cout
            << (
                printedType
                    ? ", "
                    : " "
            )
            << type
            << '='
            << transformTypeCounts[type];

        printedType = true;
    }

    if (!printedType) {
        std::cout << " (empty)";
    }

    std::cout << '\n';

    const std::size_t skeletonPreviewCount =
        std::min<std::size_t>(
            skeleton.slots.size(),
            24
        );

    for (
        std::size_t slot = 0;
        slot < skeletonPreviewCount;
        ++slot
    ) {
        const SkeletonSlotProbe& skeletonSlot =
            skeleton.slots[slot];

        std::cout
            << "  slot "
            << std::setw(3)
            << std::setfill(' ')
            << slot
            << " type="
            << std::setw(2)
            << static_cast<unsigned int>(
                skeletonSlot.transformType
            )
            << " ("
            << transformTypeName(
                skeletonSlot.transformType
            )
            << ") groups=[";

        for (
            std::size_t groupIndex = 0;
            groupIndex < skeletonSlot.groups.size();
            ++groupIndex
        ) {
            if (groupIndex != 0) {
                std::cout << ',';
            }

            std::cout
                << static_cast<unsigned int>(
                    skeletonSlot.groups[groupIndex]
                );
        }

        std::cout << "]\n";
    }

    if (
        skeleton.slots.size() >
        skeletonPreviewCount
    ) {
        std::cout
            << "  ... "
            << (
                skeleton.slots.size() -
                skeletonPreviewCount
            )
            << " more skeleton slots\n";
    }

    std::cout
        << "\nFrames\n"
        << "------------------------------\n";

    const auto printFrame =
        [&skeleton](
            std::size_t frameIndex,
            const FrameProbe& frame
        ) {
            std::cout
                << "frame["
                << frameIndex
                << "] id="
                << frame.id
                << " (0x"
                << std::hex
                << std::setw(4)
                << std::setfill('0')
                << frame.id
                << std::dec
                << std::setfill(' ')
                << ") slots="
                << static_cast<unsigned int>(
                    frame.slotCount
                )
                << " delay="
                << static_cast<unsigned int>(
                    frame.delay
                )
                << " active="
                << frame.components.size()
                << '\n';

            const std::size_t componentPreviewCount =
                std::min<std::size_t>(
                    frame.components.size(),
                    24
                );

            for (
                std::size_t componentIndex = 0;
                componentIndex < componentPreviewCount;
                ++componentIndex
            ) {
                const FrameComponentProbe& component =
                    frame.components[
                        componentIndex
                    ];

                std::cout
                    << "    slot "
                    << std::setw(3)
                    << component.slot;

                if (
                    component.slot <
                    skeleton.slots.size()
                ) {
                    std::cout
                        << " type="
                        << std::setw(2)
                        << static_cast<unsigned int>(
                            skeleton.slots[
                                component.slot
                            ].transformType
                        )
                        << " ("
                        << transformTypeName(
                            skeleton.slots[
                                component.slot
                            ].transformType
                        )
                        << ')';
                }
                else {
                    std::cout
                        << " type=??";
                }

                std::cout
                    << " flags=0x"
                    << std::hex
                    << static_cast<unsigned int>(
                        component.flags
                    )
                    << std::dec
                    << " x="
                    << optionalComponent(
                        component.x
                    )
                    << " y="
                    << optionalComponent(
                        component.y
                    )
                    << " z="
                    << optionalComponent(
                        component.z
                    )
                    << '\n';
            }

            if (
                frame.components.size() >
                componentPreviewCount
            ) {
                std::cout
                    << "    ... "
                    << (
                        frame.components.size() -
                        componentPreviewCount
                    )
                    << " more active slots\n";
            }
        };

    if (frames.size() <= 12) {
        for (
            std::size_t frameIndex = 0;
            frameIndex < frames.size();
            ++frameIndex
        ) {
            printFrame(
                frameIndex,
                frames[frameIndex]
            );
        }
    }
    else {
        constexpr std::size_t EdgeFrames = 3;

        for (
            std::size_t frameIndex = 0;
            frameIndex < EdgeFrames;
            ++frameIndex
        ) {
            printFrame(
                frameIndex,
                frames[frameIndex]
            );
        }

        std::cout
            << "  ... "
            << (
                frames.size() -
                EdgeFrames * 2
            )
            << " middle frames omitted\n";

        for (
            std::size_t frameIndex =
                frames.size() -
                EdgeFrames;
            frameIndex < frames.size();
            ++frameIndex
        ) {
            printFrame(
                frameIndex,
                frames[frameIndex]
            );
        }
    }

    std::cout
        << "\nStructural validation: PASS\n"
        << "All footer-defined sections were consumed exactly.\n";
}

void inspectAnimationFile(
    const eld::cache::Store& animationStore,
    const AnimationVersionList& versionList,
    std::uint16_t id,
    std::size_t dumpBytes
) {
    const std::optional<eld::cache::FileEntry> entry =
        animationStore.find(
            id
        );

    if (!entry.has_value()) {
        throw std::runtime_error(
            "animation file " +
            std::to_string(id) +
            " does not exist"
        );
    }

    const eld::cache::File file =
        animationStore.get(
            id
        );

    const std::vector<std::uint8_t> stored =
        file.getBytes(
            eld::cache::CompressionState::Compressed
        );

    const std::vector<std::uint8_t> decoded =
        file.getBytes();

    std::cout
        << "\nAnimation file "
        << id
        << "\n"
        << "==============================\n";

    std::cout
        << "index size:     "
        << entry->indexEntry.size
        << '\n'
        << "first sector:   "
        << entry->indexEntry.firstSector
        << '\n'
        << "sector count:   "
        << file.getPayload().getSectorCount()
        << '\n'
        << "compression:    "
        << compressionName(
            file.getCompressionType()
        )
        << '\n'
        << "stored bytes:   "
        << stored.size()
        << '\n'
        << "decoded bytes:  "
        << decoded.size()
        << '\n';

    std::cout << "version data:   ";
    printVersionMetadata(
        versionList,
        id
    );
    std::cout << '\n';

    printAnimationStructure(
        decoded
    );

    const std::size_t amount =
        std::min(
            dumpBytes,
            decoded.size()
        );

    std::cout
        << "\nDecoded head ("
        << amount
        << " bytes max)\n"
        << "------------------------------\n";

    printHexDump(
        decoded,
        0,
        dumpBytes
    );

    if (
        decoded.size() >
        dumpBytes
    ) {
        std::cout
            << "\nDecoded tail ("
            << std::min(
                dumpBytes,
                decoded.size()
            )
            << " bytes max)\n"
            << "------------------------------\n";

        printTailHexDump(
            decoded,
            dumpBytes
        );
    }

    std::cout
        << "\nASCII head\n"
        << "------------------------------\n"
        << asciiSlice(
            decoded,
            0,
            dumpBytes
        )
        << '\n';

    if (
        stored != decoded
    ) {
        std::cout
            << "\nStored/compressed head\n"
            << "------------------------------\n";

        printHexDump(
            stored,
            0,
            dumpBytes
        );

        if (
            stored.size() >
            dumpBytes
        ) {
            std::cout
                << "\nStored/compressed tail\n"
                << "------------------------------\n";

            printTailHexDump(
                stored,
                dumpBytes
            );
        }
    }

    std::cout
        << "\nProbe-only structural decode; transform application is not implemented yet.\n";
}

void validateAllAnimationFiles(
    const eld::cache::Store& animationStore
) {
    const std::vector<eld::cache::FileEntry> entries =
        animationStore.list();

    std::size_t passed = 0;
    std::size_t failed = 0;

    std::uint64_t totalFrames = 0;
    std::uint64_t totalSkeletonSlots = 0;
    std::uint64_t totalActiveTransforms = 0;

    std::size_t maxFrames = 0;
    std::uint16_t maxFramesArchive = 0;

    std::size_t maxSkeletonSlots = 0;
    std::uint16_t maxSkeletonArchive = 0;

    std::uint16_t minFrameId =
        std::numeric_limits<std::uint16_t>::max();

    std::uint16_t maxFrameId = 0;

    std::set<std::uint16_t> uniqueFrameIds;

    std::vector<std::size_t> transformTypeCounts(
        256,
        0
    );

    std::vector<std::pair<std::uint16_t, std::string>>
        failures;

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        try {
            const eld::cache::File file =
                animationStore.get(
                    entry.fileId
                );

            const std::vector<std::uint8_t> decoded =
                file.getBytes();

            const AnimationLayout layout =
                readAnimationLayout(
                    decoded
                );

            const SkeletonProbe skeleton =
                readSkeletonProbe(
                    decoded,
                    layout
                );

            const std::vector<FrameProbe> frames =
                readFrameProbes(
                    decoded,
                    layout
                );

            for (
                const FrameProbe& frame :
                frames
            ) {
                if (
                    frame.slotCount >
                    skeleton.slots.size()
                ) {
                    throw std::runtime_error(
                        "frame slot count exceeds skeleton slot count"
                    );
                }

                minFrameId =
                    std::min(
                        minFrameId,
                        frame.id
                    );

                maxFrameId =
                    std::max(
                        maxFrameId,
                        frame.id
                    );

                uniqueFrameIds.insert(
                    frame.id
                );

                totalActiveTransforms +=
                    frame.components.size();
            }

            for (
                const SkeletonSlotProbe& slot :
                skeleton.slots
            ) {
                ++transformTypeCounts[
                    slot.transformType
                ];
            }

            totalFrames +=
                frames.size();

            totalSkeletonSlots +=
                skeleton.slots.size();

            if (
                frames.size() >
                maxFrames
            ) {
                maxFrames =
                    frames.size();

                maxFramesArchive =
                    entry.fileId;
            }

            if (
                skeleton.slots.size() >
                maxSkeletonSlots
            ) {
                maxSkeletonSlots =
                    skeleton.slots.size();

                maxSkeletonArchive =
                    entry.fileId;
            }

            ++passed;
        }
        catch (const std::exception& exception) {
            ++failed;

            failures.emplace_back(
                entry.fileId,
                exception.what()
            );
        }
    }

    std::cout
        << "\nCache-wide structural validation\n"
        << "================================\n"
        << "Index 2 files:          "
        << entries.size()
        << '\n'
        << "PASS:                   "
        << passed
        << '\n'
        << "FAIL:                   "
        << failed
        << '\n'
        << "total decoded frames:   "
        << totalFrames
        << '\n'
        << "unique frame ids:       "
        << uniqueFrameIds.size()
        << '\n';

    if (!uniqueFrameIds.empty()) {
        std::cout
            << "frame id range:         "
            << minFrameId
            << " .. "
            << maxFrameId
            << '\n';
    }

    std::cout
        << "duplicate frame ids:    "
        << (
            totalFrames -
            uniqueFrameIds.size()
        )
        << '\n'
        << "total skeleton slots:   "
        << totalSkeletonSlots
        << '\n'
        << "total active transforms:"
        << ' '
        << totalActiveTransforms
        << '\n'
        << "largest frame archive:  "
        << maxFramesArchive
        << " ("
        << maxFrames
        << " frames)\n"
        << "largest skeleton:       "
        << maxSkeletonArchive
        << " ("
        << maxSkeletonSlots
        << " slots)\n";

    std::cout
        << "transform types:        ";

    bool printedType = false;

    for (
        std::size_t type = 0;
        type < transformTypeCounts.size();
        ++type
    ) {
        if (
            transformTypeCounts[type] == 0
        ) {
            continue;
        }

        if (printedType) {
            std::cout << ", ";
        }

        std::cout
            << type
            << " ("
            << transformTypeName(
                static_cast<std::uint8_t>(
                    type
                )
            )
            << ")="
            << transformTypeCounts[type];

        printedType = true;
    }

    if (!printedType) {
        std::cout << "(none)";
    }

    std::cout << '\n';

    if (!failures.empty()) {
        std::cout
            << "\nFailures\n"
            << "--------\n";

        for (
            const auto& [archiveId, message] :
            failures
        ) {
            std::cout
                << "archive "
                << archiveId
                << ": "
                << message
                << '\n';
        }
    }

    if (failed == 0) {
        std::cout
            << "\nALL INDEX 2 FILES MATCH THE PROBED STRUCTURE.\n";
    }
}

void enumerateAnimationFiles(
    const eld::cache::Store& animationStore,
    const AnimationVersionList& versionList
) {
    const std::vector<eld::cache::FileEntry> entries =
        animationStore.list();

    std::cout
        << "\nIndex 2 files\n"
        << "-------------\n"
        << "count: "
        << entries.size()
        << "\n\n";

    if (entries.empty()) {
        return;
    }

    std::uint64_t totalStored = 0;
    std::uint64_t totalDecoded = 0;

    std::size_t noneCount = 0;
    std::size_t gzipCount = 0;
    std::size_t bzip2Count = 0;
    std::size_t unknownCount = 0;

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        try {
            const eld::cache::File file =
                animationStore.get(
                    entry.fileId
                );

            const std::vector<std::uint8_t> stored =
                file.getBytes(
                    eld::cache::CompressionState::Compressed
                );

            const std::vector<std::uint8_t> decoded =
                file.getBytes();

            totalStored +=
                stored.size();

            totalDecoded +=
                decoded.size();

            switch (file.getCompressionType()) {
                case eld::binary::CompressionType::None:
                    ++noneCount;
                    break;

                case eld::binary::CompressionType::Gzip:
                    ++gzipCount;
                    break;

                case eld::binary::CompressionType::Bzip2:
                    ++bzip2Count;
                    break;

                default:
                    ++unknownCount;
                    break;
            }

            std::cout
                << "id="
                << std::setw(5)
                << std::setfill(' ')
                << entry.fileId
                << " idxSize="
                << std::setw(6)
                << entry.indexEntry.size
                << " firstSector="
                << std::setw(6)
                << entry.indexEntry.firstSector
                << " sectors="
                << std::setw(3)
                << file.getPayload().getSectorCount()
                << " comp="
                << std::setw(5)
                << compressionName(
                    file.getCompressionType()
                )
                << " stored="
                << std::setw(6)
                << stored.size()
                << " decoded="
                << std::setw(6)
                << decoded.size()
                << ' ';

            printVersionMetadata(
                versionList,
                entry.fileId
            );

            std::cout
                << "head=["
                << hexSlice(
                    decoded,
                    0,
                    SummaryHeadBytes
                )
                << "] tail=["
                << tailHex(
                    decoded,
                    SummaryTailBytes
                )
                << "]\n";
        }
        catch (const std::exception& exception) {
            std::cout
                << "id="
                << entry.fileId
                << " ERROR: "
                << exception.what()
                << '\n';
        }
    }

    std::cout
        << "\nIndex 2 summary\n"
        << "---------------\n"
        << "files:         "
        << entries.size()
        << '\n'
        << "stored bytes:  "
        << totalStored
        << '\n'
        << "decoded bytes: "
        << totalDecoded
        << '\n'
        << "compression none:    "
        << noneCount
        << '\n'
        << "compression gzip:    "
        << gzipCount
        << '\n'
        << "compression bzip2:   "
        << bzip2Count
        << '\n'
        << "compression unknown: "
        << unknownCount
        << '\n';
}

void printUsage(
    const char* executable
) {
    std::cout
        << "usage:\n"
        << "  "
        << executable
        << " <cache-root>\n"
        << "  "
        << executable
        << " <cache-root> <animation-file-id> [dump-bytes]\n"
        << "  "
        << executable
        << " <cache-root> --validate-all\n\n"
        << "examples:\n"
        << "  "
        << executable
        << " ./cache\n"
        << "  "
        << executable
        << " ./cache 0\n"
        << "  "
        << executable
        << " ./cache 42 512\n"
        << "  "
        << executable
        << " ./cache --validate-all\n";
}

int run(
    const std::filesystem::path& cacheRoot,
    const std::optional<std::uint16_t>& requestedId,
    bool validateAll,
    std::size_t dumpBytes
) {
    eld::cache::Cache cache(
        cacheRoot
    );

    const eld::cache::Store animationStore =
        cache.open(
            eld::cache::IndexId::Animations
        );

    const eld::cache::Store configStore =
        cache.open(
            eld::cache::IndexId::Config
        );

    const AnimationVersionList versionList =
        loadAnimationVersionList(
            configStore
        );

    std::cout
        << "Eldoria Animation Probe\n"
        << "=======================\n"
        << "cache: "
        << cacheRoot.string()
        << '\n'
        << "index: 2 (Animations)\n";

    printVersionListSummary(
        versionList
    );

    if (validateAll) {
        validateAllAnimationFiles(
            animationStore
        );
    }
    else if (requestedId.has_value()) {
        inspectAnimationFile(
            animationStore,
            versionList,
            *requestedId,
            dumpBytes
        );
    }
    else {
        enumerateAnimationFiles(
            animationStore,
            versionList
        );

        std::cout
            << "\nPick an interesting id and rerun with:\n"
            << "  animation_probe "
            << cacheRoot.string()
            << " <id> [dump-bytes]\n";
    }

    return 0;
}

}

int main(
    int argc,
    char** argv
) {
    try {
        if (
            argc < 2 ||
            argc > 4
        ) {
            printUsage(
                argc > 0
                    ? argv[0]
                    : "animation_probe"
            );

            return 1;
        }

        std::optional<std::uint16_t> requestedId;
        bool validateAll = false;

        if (argc >= 3) {
            const std::string_view modeOrId =
                argv[2];

            if (
                modeOrId ==
                "--validate-all"
            ) {
                validateAll = true;

                if (argc != 3) {
                    throw std::invalid_argument(
                        "--validate-all does not take dump-byte arguments"
                    );
                }
            }
            else {
                requestedId =
                    parseFileId(
                        argv[2]
                    );
            }
        }

        std::size_t dumpBytes =
            DefaultDetailBytes;

        if (
            argc >= 4 &&
            !validateAll
        ) {
            dumpBytes =
                parseAmount(
                    argv[3]
                );
        }

        return run(
            std::filesystem::path(
                argv[1]
            ),
            requestedId,
            validateAll,
            dumpBytes
        );
    }
    catch (const std::exception& exception) {
        std::cerr
            << "animation_probe failed: "
            << exception.what()
            << '\n';

        return 1;
    }
}
