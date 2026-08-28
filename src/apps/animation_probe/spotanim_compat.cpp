#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "animation/AnimationRepository.h"
#include "cache/Cache.h"
#include "cache/Index.h"
#include "definition/DefinitionRepository.h"
#include "definition/sequence/SequenceRepository.h"
#include "definition/spot_animation/SpotAnimationRepository.h"
#include "model/ModelRepository.h"

namespace {

struct FrameLocation {
    std::uint16_t archiveId = 0;
    std::size_t frameIndex = 0;
};

struct AnimationIndex {
    std::map<std::uint16_t, eld::animation::Animation> archives;
    std::map<std::uint16_t, FrameLocation> frames;
};

struct ModelGroups {
    std::array<std::size_t, 256> vertexCounts {};
    std::array<std::size_t, 256> faceCounts {};

    std::size_t skinnedVertices = 0;
    std::size_t skinnedFaces = 0;
};

struct Compatibility {
    std::set<std::uint8_t> referencedVertexGroups;
    std::set<std::uint8_t> missingVertexGroups;

    std::set<std::uint8_t> referencedFaceGroups;
    std::set<std::uint8_t> missingFaceGroups;

    std::set<std::uint8_t> type4Groups;

    std::size_t resolvedFrames = 0;
    std::size_t missingFrames = 0;
    std::size_t explicitTransforms = 0;
    std::size_t invalidSlots = 0;

    std::set<std::uint16_t> archivesUsed;
};

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
        case 4:
            return "unknown4";
        case 5:
            return "alpha";
        default:
            return "unknown";
    }
}

AnimationIndex buildAnimationIndex(
    const eld::animation::AnimationRepository& repository
) {
    AnimationIndex index;

    for (
        const std::uint16_t archiveId :
        repository.listIds()
    ) {
        eld::animation::Animation animation =
            repository.get(
                archiveId
            );

        for (
            std::size_t frameIndex = 0;
            frameIndex < animation.asset.frames.size();
            ++frameIndex
        ) {
            const std::uint16_t frameId =
                animation.asset.frames[frameIndex].id;

            const auto [iterator, inserted] =
                index.frames.emplace(
                    frameId,
                    FrameLocation{
                        archiveId,
                        frameIndex
                    }
                );

            if (!inserted) {
                throw std::runtime_error(
                    "Duplicate global animation frame id " +
                    std::to_string(
                        frameId
                    )
                );
            }

            (void) iterator;
        }

        index.archives.emplace(
            archiveId,
            std::move(
                animation
            )
        );
    }

    return index;
}

const eld::animation::AnimationFrame* resolveFrame(
    const AnimationIndex& index,
    std::uint16_t frameId,
    const eld::animation::Skeleton** skeleton,
    std::uint16_t* archiveId
) {
    const auto locationIterator =
        index.frames.find(
            frameId
        );

    if (
        locationIterator ==
        index.frames.end()
    ) {
        return nullptr;
    }

    const FrameLocation& location =
        locationIterator->second;

    const auto archiveIterator =
        index.archives.find(
            location.archiveId
        );

    if (
        archiveIterator ==
        index.archives.end()
    ) {
        throw std::runtime_error(
            "Frame index references an unloaded animation archive"
        );
    }

    const eld::animation::Animation& animation =
        archiveIterator->second;

    if (
        location.frameIndex >=
        animation.asset.frames.size()
    ) {
        throw std::runtime_error(
            "Frame index points outside its animation archive"
        );
    }

    if (skeleton != nullptr) {
        *skeleton =
            &animation.asset.skeleton;
    }

    if (archiveId != nullptr) {
        *archiveId =
            location.archiveId;
    }

    return
        &animation.asset.frames[
            location.frameIndex
        ];
}

ModelGroups buildModelGroups(
    const eld::model::ModelMesh& mesh
) {
    ModelGroups groups;

    for (
        const eld::model::Vertex& vertex :
        mesh.vertices
    ) {
        if (!vertex.skin.has_value()) {
            continue;
        }

        ++groups.skinnedVertices;
        ++groups.vertexCounts[*vertex.skin];
    }

    for (
        const eld::model::Face& face :
        mesh.faces
    ) {
        if (!face.skin.has_value()) {
            continue;
        }

        ++groups.skinnedFaces;
        ++groups.faceCounts[*face.skin];
    }

    return groups;
}

void printGroupCounts(
    std::string_view label,
    const std::array<std::size_t, 256>& counts
) {
    std::cout << label << '\n';

    bool any = false;

    for (
        std::size_t group = 0;
        group < counts.size();
        ++group
    ) {
        if (counts[group] == 0) {
            continue;
        }

        any = true;

        std::cout
            << "  "
            << group
            << " -> "
            << counts[group]
            << '\n';
    }

    if (!any) {
        std::cout << "  (none)\n";
    }
}

template <typename T>
void printSet(
    std::string_view label,
    const std::set<T>& values
) {
    std::cout << label;

    if (values.empty()) {
        std::cout << " none\n";
        return;
    }

    bool first = true;

    for (
        const T value :
        values
    ) {
        if (!first) {
            std::cout << ',';
        }

        std::cout
            << ' '
            << static_cast<unsigned int>(
                value
            );

        first = false;
    }

    std::cout << '\n';
}

void addFrameCompatibility(
    Compatibility& compatibility,
    const ModelGroups& modelGroups,
    const eld::animation::AnimationFrame& frame,
    const eld::animation::Skeleton& skeleton,
    std::uint16_t archiveId
) {
    ++compatibility.resolvedFrames;

    compatibility.archivesUsed.insert(
        archiveId
    );

    for (
        const eld::animation::FrameTransform& transform :
        frame.transforms
    ) {
        ++compatibility.explicitTransforms;

        if (
            transform.slot >=
            skeleton.slots.size()
        ) {
            ++compatibility.invalidSlots;
            continue;
        }

        const eld::animation::SkeletonSlot& slot =
            skeleton.slots[
                transform.slot
            ];

        if (slot.type == 5) {
            for (
                const std::uint8_t group :
                slot.groups
            ) {
                compatibility.referencedFaceGroups.insert(
                    group
                );

                if (
                    modelGroups.faceCounts[group] ==
                    0
                ) {
                    compatibility.missingFaceGroups.insert(
                        group
                    );
                }
            }

            continue;
        }

        if (slot.type == 4) {
            for (
                const std::uint8_t group :
                slot.groups
            ) {
                compatibility.type4Groups.insert(
                    group
                );
            }

            continue;
        }

        for (
            const std::uint8_t group :
            slot.groups
        ) {
            compatibility.referencedVertexGroups.insert(
                group
            );

            if (
                modelGroups.vertexCounts[group] ==
                0
            ) {
                compatibility.missingVertexGroups.insert(
                    group
                );
            }
        }
    }
}

Compatibility inspectSequenceCompatibility(
    const eld::definition::SequenceDefinition& sequence,
    const AnimationIndex& animationIndex,
    const ModelGroups& modelGroups
) {
    Compatibility compatibility;

    for (
        const eld::definition::SequenceFrame& sequenceFrame :
        sequence.frames
    ) {
        const eld::animation::Skeleton* skeleton =
            nullptr;

        std::uint16_t archiveId = 0;

        const eld::animation::AnimationFrame* frame =
            resolveFrame(
                animationIndex,
                sequenceFrame.primaryFrameId,
                &skeleton,
                &archiveId
            );

        if (
            frame == nullptr ||
            skeleton == nullptr
        ) {
            ++compatibility.missingFrames;
            continue;
        }

        addFrameCompatibility(
            compatibility,
            modelGroups,
            *frame,
            *skeleton,
            archiveId
        );
    }

    return compatibility;
}

bool isUsableAutomaticCandidate(
    const eld::definition::SpotAnimationDefinition& spot,
    const eld::definition::SequenceRepository& sequences,
    const eld::model::ModelRepository& models,
    const AnimationIndex& animationIndex
) {
    if (
        !spot.modelId.has_value() ||
        !spot.sequenceId.has_value()
    ) {
        return false;
    }

    const eld::definition::SequenceDefinition* sequence =
        sequences.find(
            *spot.sequenceId
        );

    if (
        sequence == nullptr ||
        sequence->frames.empty()
    ) {
        return false;
    }

    if (
        !models.contains(
            *spot.modelId
        )
    ) {
        return false;
    }

    for (
        const eld::definition::SequenceFrame& frame :
        sequence->frames
    ) {
        if (
            animationIndex.frames.find(
                frame.primaryFrameId
            ) ==
            animationIndex.frames.end()
        ) {
            return false;
        }
    }

    const eld::model::Model model =
        models.get(
            *spot.modelId
        );

    const ModelGroups groups =
        buildModelGroups(
            model.mesh
        );

    return
        groups.skinnedVertices > 0 ||
        groups.skinnedFaces > 0;
}

const eld::definition::SpotAnimationDefinition*
selectSpotAnimation(
    const eld::definition::SpotAnimationRepository& spots,
    const eld::definition::SequenceRepository& sequences,
    const eld::model::ModelRepository& models,
    const AnimationIndex& animationIndex,
    const std::optional<std::uint16_t>& requestedId
) {
    if (requestedId.has_value()) {
        return spots.find(
            *requestedId
        );
    }

    for (
        const eld::definition::SpotAnimationDefinition& spot :
        spots.list()
    ) {
        try {
            if (
                isUsableAutomaticCandidate(
                    spot,
                    sequences,
                    models,
                    animationIndex
                )
            ) {
                return &spot;
            }
        }
        catch (
            const std::exception&
        ) {
            continue;
        }
    }

    return nullptr;
}

void printFrameDetails(
    const eld::definition::SequenceFrame& sequenceFrame,
    const AnimationIndex& animationIndex,
    const ModelGroups& modelGroups
) {
    const eld::animation::Skeleton* skeleton =
        nullptr;

    std::uint16_t archiveId = 0;

    const eld::animation::AnimationFrame* frame =
        resolveFrame(
            animationIndex,
            sequenceFrame.primaryFrameId,
            &skeleton,
            &archiveId
        );

    std::cout
        << "\nFirst sequence frame\n"
        << "--------------------\n"
        << "global frame id:      "
        << sequenceFrame.primaryFrameId
        << '\n'
        << "sequence duration:    "
        << sequenceFrame.duration
        << '\n';

    if (
        frame == nullptr ||
        skeleton == nullptr
    ) {
        std::cout
            << "resolution:           MISSING\n";

        return;
    }

    std::cout
        << "animation archive:    "
        << archiveId
        << '\n'
        << "frame delay byte:     "
        << static_cast<unsigned int>(
            frame->delay
        )
        << '\n'
        << "frame slotCount:      "
        << static_cast<unsigned int>(
            frame->slotCount
        )
        << '\n'
        << "skeleton slots:       "
        << skeleton->slots.size()
        << '\n'
        << "explicit transforms:  "
        << frame->transforms.size()
        << '\n';

    std::cout
        << "\nTransforms\n"
        << "----------\n";

    constexpr std::size_t MaxPrintedTransforms = 40;

    std::size_t printed = 0;

    for (
        const eld::animation::FrameTransform& transform :
        frame->transforms
    ) {
        if (
            printed >=
            MaxPrintedTransforms
        ) {
            break;
        }

        ++printed;

        std::cout
            << "slot "
            << transform.slot;

        if (
            transform.slot >=
            skeleton->slots.size()
        ) {
            std::cout
                << " INVALID\n";

            continue;
        }

        const eld::animation::SkeletonSlot& slot =
            skeleton->slots[
                transform.slot
            ];

        std::cout
            << "  type="
            << static_cast<unsigned int>(
                slot.type
            )
            << " ("
            << transformTypeName(
                slot.type
            )
            << ")"
            << " flags=0x"
            << std::hex
            << std::setw(2)
            << std::setfill('0')
            << static_cast<unsigned int>(
                transform.flags
            )
            << std::dec
            << std::setfill(' ')
            << " xyz=("
            << transform.x
            << ','
            << transform.y
            << ','
            << transform.z
            << ")"
            << " groups=[";

        bool first = true;

        for (
            const std::uint8_t group :
            slot.groups
        ) {
            if (!first) {
                std::cout << ',';
            }

            std::cout
                << static_cast<unsigned int>(
                    group
                );

            first = false;
        }

        std::cout << ']';

        if (slot.type == 5) {
            std::cout
                << " faceHits={";

            first = true;

            for (
                const std::uint8_t group :
                slot.groups
            ) {
                if (!first) {
                    std::cout << ',';
                }

                std::cout
                    << static_cast<unsigned int>(
                        group
                    )
                    << ':'
                    << modelGroups.faceCounts[group];

                first = false;
            }

            std::cout << '}';
        }
        else if (slot.type == 4) {
            std::cout
                << " compatibility=UNKNOWN_TYPE4";
        }
        else {
            std::cout
                << " vertexHits={";

            first = true;

            for (
                const std::uint8_t group :
                slot.groups
            ) {
                if (!first) {
                    std::cout << ',';
                }

                std::cout
                    << static_cast<unsigned int>(
                        group
                    )
                    << ':'
                    << modelGroups.vertexCounts[group];

                first = false;
            }

            std::cout << '}';
        }

        std::cout << '\n';
    }

    if (
        frame->transforms.size() >
        MaxPrintedTransforms
    ) {
        std::cout
            << "... "
            << (
                frame->transforms.size() -
                MaxPrintedTransforms
            )
            << " more transforms omitted\n";
    }
}

int run(
    const std::filesystem::path& cacheRoot,
    const std::optional<std::uint16_t>& requestedSpotId
) {
    eld::cache::Cache cache(
        cacheRoot
    );

    eld::definition::DefinitionRepository definitions(
        cache.open(
            eld::cache::IndexId::Config
        ),
        2
    );

    eld::definition::SequenceRepository sequences(
        definitions.get(
            "seq"
        )
    );

    eld::definition::SpotAnimationRepository spots(
        definitions.get(
            "spotanim"
        )
    );

    eld::model::ModelRepository models(
        cache.open(
            eld::cache::IndexId::Models
        )
    );

    eld::animation::AnimationRepository animations(
        cache.open(
            eld::cache::IndexId::Animations
        )
    );

    std::cout
        << "Eldoria SpotAnim / Animation Compatibility Probe\n"
        << "===============================================\n"
        << "building global frame index...\n";

    const AnimationIndex animationIndex =
        buildAnimationIndex(
            animations
        );

    std::cout
        << "animation archives:    "
        << animationIndex.archives.size()
        << '\n'
        << "global frames indexed: "
        << animationIndex.frames.size()
        << '\n';

    const eld::definition::SpotAnimationDefinition* spot =
        selectSpotAnimation(
            spots,
            sequences,
            models,
            animationIndex,
            requestedSpotId
        );

    if (spot == nullptr) {
        if (requestedSpotId.has_value()) {
            std::cerr
                << "spot animation "
                << *requestedSpotId
                << " was not found or could not be inspected\n";
        }
        else {
            std::cerr
                << "no automatic SpotAnim candidate with model + sequence + skins was found\n";
        }

        return 1;
    }

    if (
        !spot->modelId.has_value() ||
        !spot->sequenceId.has_value()
    ) {
        std::cerr
            << "spot animation "
            << spot->id
            << " does not contain both modelId and sequenceId\n";

        return 1;
    }

    if (!requestedSpotId.has_value()) {
        std::cout
            << "auto-selected SpotAnim "
            << spot->id
            << '\n';
    }

    const eld::definition::SequenceDefinition* sequence =
        sequences.find(
            *spot->sequenceId
        );

    if (sequence == nullptr) {
        std::cerr
            << "sequence "
            << *spot->sequenceId
            << " does not exist\n";

        return 1;
    }

    const std::optional<eld::model::Model> modelOptional =
        models.find(
            *spot->modelId
        );

    if (!modelOptional.has_value()) {
        std::cerr
            << "model "
            << *spot->modelId
            << " does not exist\n";

        return 1;
    }

    const eld::model::Model& model =
        *modelOptional;

    const ModelGroups modelGroups =
        buildModelGroups(
            model.mesh
        );

    const Compatibility compatibility =
        inspectSequenceCompatibility(
            *sequence,
            animationIndex,
            modelGroups
        );

    std::cout
        << "\nSpot animation\n"
        << "--------------\n"
        << "id:                   "
        << spot->id
        << '\n'
        << "modelId:              "
        << *spot->modelId
        << '\n'
        << "sequenceId:           "
        << *spot->sequenceId
        << '\n'
        << "scaleX:               "
        << spot->scaleX
        << '\n'
        << "scaleY:               "
        << spot->scaleY
        << '\n'
        << "rotation:             "
        << spot->rotation
        << '\n';

    std::cout
        << "\nModel\n"
        << "-----\n"
        << "vertices:             "
        << model.mesh.vertices.size()
        << '\n'
        << "faces:                "
        << model.mesh.faces.size()
        << '\n'
        << "skinned vertices:     "
        << modelGroups.skinnedVertices
        << '\n'
        << "skinned faces:        "
        << modelGroups.skinnedFaces
        << '\n';

    printGroupCounts(
        "vertex skin groups:",
        modelGroups.vertexCounts
    );

    printGroupCounts(
        "face skin groups:",
        modelGroups.faceCounts
    );

    std::cout
        << "\nSequence\n"
        << "--------\n"
        << "id:                   "
        << sequence->id
        << '\n'
        << "frames:               "
        << sequence->frames.size()
        << '\n'
        << "frameStep:            ";

    if (sequence->frameStep.has_value()) {
        std::cout
            << *sequence->frameStep;
    }
    else {
        std::cout
            << "none";
    }

    std::cout
        << '\n'
        << "interleave entries:   "
        << sequence->interleaveOrder.size()
        << '\n';

    const std::size_t previewFrameCount =
        std::min<std::size_t>(
            sequence->frames.size(),
            16
        );

    for (
        std::size_t i = 0;
        i < previewFrameCount;
        ++i
    ) {
        const eld::definition::SequenceFrame& frame =
            sequence->frames[i];

        const auto location =
            animationIndex.frames.find(
                frame.primaryFrameId
            );

        std::cout
            << "  ["
            << i
            << "] frame="
            << frame.primaryFrameId
            << " duration="
            << frame.duration;

        if (
            location !=
            animationIndex.frames.end()
        ) {
            std::cout
                << " archive="
                << location->second.archiveId;
        }
        else {
            std::cout
                << " MISSING";
        }

        if (frame.secondaryFrameId.has_value()) {
            std::cout
                << " secondary="
                << *frame.secondaryFrameId;
        }

        std::cout << '\n';
    }

    if (
        sequence->frames.size() >
        previewFrameCount
    ) {
        std::cout
            << "  ... "
            << (
                sequence->frames.size() -
                previewFrameCount
            )
            << " more sequence frames omitted\n";
    }

    std::cout
        << "\nCompatibility\n"
        << "-------------\n"
        << "resolved sequence frames: "
        << compatibility.resolvedFrames
        << " / "
        << sequence->frames.size()
        << '\n'
        << "missing sequence frames:  "
        << compatibility.missingFrames
        << '\n'
        << "animation archives used:  "
        << compatibility.archivesUsed.size()
        << '\n'
        << "explicit transforms:      "
        << compatibility.explicitTransforms
        << '\n'
        << "invalid skeleton slots:   "
        << compatibility.invalidSlots
        << '\n'
        << "vertex groups referenced: "
        << compatibility.referencedVertexGroups.size()
        << '\n'
        << "vertex groups missing:    "
        << compatibility.missingVertexGroups.size()
        << '\n'
        << "face groups referenced:   "
        << compatibility.referencedFaceGroups.size()
        << '\n'
        << "face groups missing:      "
        << compatibility.missingFaceGroups.size()
        << '\n'
        << "type-4 groups observed:   "
        << compatibility.type4Groups.size()
        << '\n';

    printSet(
        "missing vertex group ids:",
        compatibility.missingVertexGroups
    );

    printSet(
        "missing face group ids:",
        compatibility.missingFaceGroups
    );

    printSet(
        "type-4 group ids:",
        compatibility.type4Groups
    );

    if (!sequence->frames.empty()) {
        printFrameDetails(
            sequence->frames.front(),
            animationIndex,
            modelGroups
        );
    }

    const bool clean =
        compatibility.missingFrames == 0 &&
        compatibility.invalidSlots == 0 &&
        compatibility.missingVertexGroups.empty() &&
        compatibility.missingFaceGroups.empty();

    std::cout
        << "\nResult\n"
        << "------\n";

    if (clean) {
        std::cout
            << "MODEL/SKELETON GROUP COMPATIBILITY: CLEAN\n"
            << "Safe next experiment: apply one frame in the probe.\n";

        return 0;
    }

    std::cout
        << "MODEL/SKELETON GROUP COMPATIBILITY: HAS GAPS\n"
        << "Do not deform the model yet; inspect the missing references above.\n";

    return 2;
}

std::optional<std::uint16_t> parseSpotId(
    const char* value
) {
    try {
        const unsigned long parsed =
            std::stoul(
                value
            );

        if (
            parsed >
            65535UL
        ) {
            return std::nullopt;
        }

        return
            static_cast<std::uint16_t>(
                parsed
            );
    }
    catch (...) {
        return std::nullopt;
    }
}

}

int main(
    int argc,
    char** argv
) {
    if (
        argc < 2 ||
        argc > 3
    ) {
        std::cerr
            << "usage: "
            << (
                argc > 0
                    ? argv[0]
                    : "animation_spotanim_probe"
            )
            << " <cache-root> [spotanim-id]\n"
            << '\n'
            << "With no spotanim-id, the probe selects the first complete\n"
            << "SpotAnim -> model -> sequence -> animation chain that has\n"
            << "model skin metadata.\n";

        return 1;
    }

    std::optional<std::uint16_t> spotId;

    if (
        argc ==
        3
    ) {
        spotId =
            parseSpotId(
                argv[2]
            );

        if (!spotId.has_value()) {
            std::cerr
                << "invalid spotanim id: "
                << argv[2]
                << '\n';

            return 1;
        }
    }

    try {
        return run(
            std::filesystem::path(
                argv[1]
            ),
            spotId
        );
    }
    catch (
        const std::exception& exception
    ) {
        std::cerr
            << "animation_spotanim_probe failed: "
            << exception.what()
            << '\n';

        return 1;
    }
}

