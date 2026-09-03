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
                << animation.frames.size()
                << "\n"
                << "Skeleton slots: "
                << animation.skeleton.size()
                << "\n"
                << "Sequence references: "
                << inspection.sequences.size()
                << "\n"
                << "Known uses: "
                << inspection.uses.size()
                << "\n\n";

            output << "[Skeleton]\n";

            for (
                std::size_t index = 0;
                index <
                    animation.skeleton.size();
                ++index
            ) {
                const eld::animation::SkeletonSlot& slot =
                    animation.skeleton[index];

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
                index < animation.frames.size();
                ++index
            ) {
                const eld::animation::AnimationFrame& frame =
                    animation.frames[index];

                output
                    << "Frame " << index << "\n"
                    << "  Global ID: "
                    << frame.id
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

        },
        error
    );
}

}
