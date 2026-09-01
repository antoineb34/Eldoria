#include "dump/AnimationDumper.h"

#include <ostream>

#include "dump/AssetDumpWriter.h"

namespace eld::elforge {

std::filesystem::path defaultAnimationDumpPath(
    std::uint16_t animationId
) {
    return
        std::filesystem::path("dumps") /
        "animation" /
        (
            "animation_" +
            std::to_string(animationId) +
            ".txt"
        );
}


bool dumpAnimation(
    const AnimationInspection& inspection,
    const std::filesystem::path& path,
    std::string& error
) {
    return writeAssetDump(
        path,
        [&inspection](std::ostream& output) {
            const eld::animation::Animation& animation =
                inspection.animation;

            output
                << "ELDORIA ASSET DUMP\n"
                << "Type: Animation\n"
                << "ID: " << animation.id << "\n\n";

            output
                << "[Summary]\n"
                << "Frames: "
                << animation.asset.frames.size()
                << "\n"
                << "Skeleton slots: "
                << animation.asset.skeleton.slots.size()
                << "\n"
                << "Sequence references: "
                << inspection.sequences.size()
                << "\n"
                << "Known uses: "
                << inspection.uses.size()
                << "\n"
                << "Raw file payload bytes: "
                << animation.file.bytes.size()
                << "\n\n";

            output << "[Skeleton]\n";

            for (
                std::size_t index = 0;
                index <
                    animation.asset.skeleton.slots.size();
                ++index
            ) {
                const eld::animation::SkeletonSlot& slot =
                    animation.asset.skeleton.slots[index];

                output
                    << "Slot " << index
                    << "\n"
                    << "  Type: "
                    << static_cast<unsigned int>(
                        slot.type
                    )
                    << "\n"
                    << "  Groups:";

                if (slot.groups.empty()) {
                    output << " (none)";
                }
                else {
                    for (const auto group : slot.groups) {
                        output
                            << ' '
                            << static_cast<unsigned int>(
                                group
                            );
                    }
                }

                output << "\n";
            }

            output << "\n[Frames]\n";

            for (
                std::size_t index = 0;
                index < animation.asset.frames.size();
                ++index
            ) {
                const eld::animation::AnimationFrame& frame =
                    animation.asset.frames[index];

                output
                    << "Frame " << index << "\n"
                    << "  Global ID: "
                    << frame.id
                    << "\n"
                    << "  Slot count: "
                    << static_cast<unsigned int>(
                        frame.slotCount
                    )
                    << "\n"
                    << "  Delay: "
                    << static_cast<unsigned int>(
                        frame.delay
                    )
                    << "\n"
                    << "  Transform count: "
                    << frame.transforms.size()
                    << "\n";

                for (
                    std::size_t transformIndex = 0;
                    transformIndex <
                        frame.transforms.size();
                    ++transformIndex
                ) {
                    const eld::animation::FrameTransform&
                        transform =
                            frame.transforms[
                                transformIndex
                            ];

                    output
                        << "    Transform "
                        << transformIndex
                        << ": slot="
                        << transform.slot
                        << " flags=0x"
                        << std::hex
                        << static_cast<unsigned int>(
                            transform.flags
                        )
                        << std::dec
                        << " x=" << transform.x
                        << " y=" << transform.y
                        << " z=" << transform.z
                        << "\n";
                }
            }

            output << "\n[Sequence References]\n";

            if (inspection.sequences.empty()) {
                output << "(none)\n";
            }

            for (
                const AnimationSequenceReference& sequence :
                inspection.sequences
            ) {
                output
                    << "Sequence "
                    << sequence.sequenceId
                    << "\n"
                    << "  Matching primary frames: "
                    << sequence.matchingPrimaryFrames
                    << "\n"
                    << "  Matching secondary frames: "
                    << sequence.matchingSecondaryFrames
                    << "\n"
                    << "  Total frame references: "
                    << sequence.totalFrameReferences
                    << "\n";
            }

            output << "\n[Known Uses / Relationships]\n";

            if (inspection.uses.empty()) {
                output << "(none)\n";
            }

            for (
                const AnimationUse& use :
                inspection.uses
            ) {
                output
                    << use.source
                    << ' '
                    << use.sourceId;

                if (!use.sourceName.empty()) {
                    output
                        << " - "
                        << use.sourceName;
                }

                output
                    << "\n"
                    << "  Role: "
                    << use.role
                    << "\n"
                    << "  Sequence: "
                    << use.sequenceId
                    << "\n";

                if (use.viaSpotAnimationId.has_value()) {
                    output
                        << "  Via spot animation: "
                        << *use.viaSpotAnimationId
                        << "\n";
                }

                output
                    << "  Provenance: "
                    << use.provenance
                    << "\n";
            }

            output
                << "\n[Raw File Bytes]\n"
                << "This is the exact byte buffer stored in "
                   "AnimationFile and passed through the "
                   "animation decoding pipeline.\n"
                << "Size: "
                << animation.file.bytes.size()
                << " bytes\n\n";

            if (animation.file.bytes.empty()) {
                output << "(empty)\n";
            }
            else {
                writeHexDump(
                    output,
                    animation.file.bytes.data(),
                    animation.file.bytes.size()
                );
            }
        },
        error
    );
}

}
