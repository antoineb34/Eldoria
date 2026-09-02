#include "animation/AnimationPlayer.h"
#include "inspection/AnimationInspector.h"

#include <algorithm>
#include <set>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>

#include "animation/presentation/AnimationAction.h"

namespace eld::elforge {

namespace {

std::uint64_t sequenceDurationMilliseconds(
    const eld::animation::AnimationFrameIndex& frames,
    const eld::definition::SequenceDefinition& sequence
) {
    eld::render::AnimationPlayer player(
        frames
    );

    player.setSequence(
        sequence
    );

    player.setLooping(
        false
    );

    player.pause();

    std::uint64_t total = 0;

    const std::size_t frameCount =
        player.frameCount();

    for (
        std::size_t index = 0;
        index < frameCount;
        ++index
    ) {
        total +=
            player.
                currentFrameDurationMilliseconds();

        if (index + 1 < frameCount) {
            player.stepForward();
        }
    }

    return total;
}


using eld::animation::presentation::AnimationAction;
using eld::animation::presentation::AnimationBinding;

struct SequenceArchiveUsage {
    std::size_t primary = 0;
    std::size_t secondary = 0;
    std::size_t totalReferences = 0;
};

SequenceArchiveUsage sequenceArchiveUsage(
    const eld::definition::SequenceDefinition& sequence,
    std::uint16_t animationId,
    const eld::animation::AnimationFrameIndex& frames
) {
    SequenceArchiveUsage usage;

    for (const eld::definition::SequenceFrame& frame : sequence.frames) {
        ++usage.totalReferences;

        const eld::animation::ResolvedAnimationFrame primary =
            frames.resolve(frame.primaryFrameId);

        if (
            primary &&
            primary.archiveId == animationId
        ) {
            ++usage.primary;
        }

        if (frame.secondaryFrameId.has_value()) {
            ++usage.totalReferences;

            const eld::animation::ResolvedAnimationFrame secondary =
                frames.resolve(*frame.secondaryFrameId);

            if (
                secondary &&
                secondary.archiveId == animationId
            ) {
                ++usage.secondary;
            }
        }
    }

    return usage;
}

std::optional<std::uint16_t> cacheNpcSequenceForAction(
    const eld::definition::NpcDefinition& npc,
    AnimationAction action
) {
    switch (action) {
        case AnimationAction::Idle:
            return npc.idleAnimationId;
        case AnimationAction::Walk:
            return npc.walkAnimationId;
        case AnimationAction::TurnAround:
            return npc.turnAroundAnimationId;
        case AnimationAction::TurnLeft:
            return npc.turnLeftAnimationId;
        case AnimationAction::TurnRight:
            return npc.turnRightAnimationId;
        default:
            return std::nullopt;
    }
}

bool sequenceMatches(
    const std::unordered_set<std::uint16_t>& sequenceIds,
    const std::optional<std::uint16_t>& sequenceId
) {
    return
        sequenceId.has_value() &&
        sequenceIds.contains(*sequenceId);
}

std::string usableName(
    const std::string& name
) {
    return name == "null"
        ? std::string{}
        : name;
}

void addUse(
    std::vector<AnimationUse>& uses,
    AnimationUse use
) {
    const auto duplicate =
        std::find_if(
            uses.begin(),
            uses.end(),
            [&](const AnimationUse& existing) {
                return
                    existing.source == use.source &&
                    existing.sourceId == use.sourceId &&
                    existing.role == use.role &&
                    existing.sequenceId == use.sequenceId &&
                    existing.viaSpotAnimationId == use.viaSpotAnimationId &&
                    existing.provenance == use.provenance;
            }
        );

    if (duplicate == uses.end()) {
        uses.push_back(std::move(use));
    }
}

void addNpcCacheUse(
    std::vector<AnimationUse>& uses,
    const std::unordered_set<std::uint16_t>& sequenceIds,
    const eld::definition::NpcDefinition& npc,
    const std::optional<std::uint16_t>& sequenceId,
    const char* role
) {
    if (!sequenceMatches(sequenceIds, sequenceId)) {
        return;
    }

    addUse(
        uses,
        AnimationUse{
            .source = "NPC",
            .sourceId = npc.id,
            .sourceName = usableName(npc.name),
            .role = role,
            .sequenceId = *sequenceId,
            .viaSpotAnimationId = std::nullopt,
            .provenance = "cache"
        }
    );
}

void addPresentationBindingUses(
    std::vector<AnimationUse>& uses,
    const std::unordered_set<std::uint16_t>& sequenceIds,
    const eld::definition::SpotAnimationRepository& spots,
    const std::string& source,
    std::uint16_t sourceId,
    const std::string& sourceName,
    const AnimationBinding& binding,
    const std::optional<std::uint16_t>& cacheSequence
) {
    if (
        binding.sequenceId.has_value() &&
        sequenceIds.contains(*binding.sequenceId) &&
        binding.sequenceId != cacheSequence
    ) {
        addUse(
            uses,
            AnimationUse{
                .source = source,
                .sourceId = sourceId,
                .sourceName = sourceName,
                .role = std::string(
                    eld::animation::presentation::toString(
                        binding.action
                    )
                ),
                .sequenceId = *binding.sequenceId,
                .viaSpotAnimationId = std::nullopt,
                .provenance = "eldoria_content"
            }
        );
    }

    for (
        const eld::animation::presentation::AnimationEffectBinding& effect :
        binding.effects
    ) {
        const eld::definition::SpotAnimationDefinition* spot =
            spots.find(effect.spotAnimationId);

        if (
            spot == nullptr ||
            !sequenceMatches(sequenceIds, spot->sequenceId)
        ) {
            continue;
        }

        addUse(
            uses,
            AnimationUse{
                .source = source,
                .sourceId = sourceId,
                .sourceName = sourceName,
                .role = std::string(
                    eld::animation::presentation::toString(
                        binding.action
                    )
                ) + " effect",
                .sequenceId = *spot->sequenceId,
                .viaSpotAnimationId = effect.spotAnimationId,
                .provenance = "eldoria_content"
            }
        );
    }
}

}

AnimationInspector::AnimationInspector(
    const eld::animation::AnimationRepository& animations,
    const eld::animation::AnimationFrameIndex& frames,
    const eld::definition::SequenceRepository& sequences,
    const eld::definition::NpcRepository& npcs,
    const eld::definition::LocationRepository& locations,
    const eld::definition::SpotAnimationRepository& spotAnimations,
    const eld::definition::ItemRepository& items,
    const eld::interface::InterfaceRepository& interfaces,
    const eld::animation::presentation::AnimationPresentationCatalog& presentation
)
    : animations_(&animations),
      frames_(&frames),
      sequences_(&sequences),
      npcs_(&npcs),
      locations_(&locations),
      spotAnimations_(&spotAnimations),
      items_(&items),
      interfaces_(&interfaces),
      presentation_(&presentation) {
}

AnimationInspection AnimationInspector::inspect(
    std::uint16_t animationId
) const {
    AnimationInspection info;
    info.animation = animations_->get(animationId);

    std::unordered_set<std::uint16_t> referencedSequenceIds;

    for (
        const eld::definition::SequenceDefinition& sequence :
        sequences_->list()
    ) {
        const SequenceArchiveUsage usage =
            sequenceArchiveUsage(
                sequence,
                animationId,
                *frames_
            );

        if (usage.primary == 0 && usage.secondary == 0) {
            continue;
        }

        referencedSequenceIds.insert(sequence.id);

        info.sequences.push_back({
            .sequenceId = sequence.id,
            .matchingPrimaryFrames = usage.primary,
            .matchingSecondaryFrames = usage.secondary,
            .totalFrameReferences = usage.totalReferences,
            .durationMilliseconds =
                sequenceDurationMilliseconds(
                    *frames_,
                    sequence
                )
        });
    }

    for (const eld::definition::NpcDefinition& npc : npcs_->list()) {
        addNpcCacheUse(
            info.uses,
            referencedSequenceIds,
            npc,
            npc.idleAnimationId,
            "Idle"
        );

        addNpcCacheUse(
            info.uses,
            referencedSequenceIds,
            npc,
            npc.walkAnimationId,
            "Walk"
        );

        addNpcCacheUse(
            info.uses,
            referencedSequenceIds,
            npc,
            npc.turnAroundAnimationId,
            "Turn around"
        );

        addNpcCacheUse(
            info.uses,
            referencedSequenceIds,
            npc,
            npc.turnLeftAnimationId,
            "Turn left"
        );

        addNpcCacheUse(
            info.uses,
            referencedSequenceIds,
            npc,
            npc.turnRightAnimationId,
            "Turn right"
        );

        const eld::animation::presentation::NpcAnimationProfile profile =
            presentation_->resolveNpc(npc);

        for (const AnimationBinding& binding : profile.bindings) {
            addPresentationBindingUses(
                info.uses,
                referencedSequenceIds,
                *spotAnimations_,
                "NPC",
                npc.id,
                usableName(npc.name),
                binding,
                cacheNpcSequenceForAction(npc, binding.action)
            );
        }
    }

    for (
        const eld::definition::LocationDefinition& location :
        locations_->list()
    ) {
        if (
            !sequenceMatches(
                referencedSequenceIds,
                location.animationId
            )
        ) {
            continue;
        }

        addUse(
            info.uses,
            AnimationUse{
                .source = "Location",
                .sourceId = location.id,
                .sourceName = usableName(location.name),
                .role = "Animation",
                .sequenceId = *location.animationId,
                .viaSpotAnimationId = std::nullopt,
                .provenance = "cache"
            }
        );
    }

    for (
        const eld::definition::SpotAnimationDefinition& spot :
        spotAnimations_->list()
    ) {
        if (
            !sequenceMatches(
                referencedSequenceIds,
                spot.sequenceId
            )
        ) {
            continue;
        }

        addUse(
            info.uses,
            AnimationUse{
                .source = "SpotAnim",
                .sourceId = spot.id,
                .sourceName = {},
                .role = "Animation",
                .sequenceId = *spot.sequenceId,
                .viaSpotAnimationId = std::nullopt,
                .provenance = "cache"
            }
        );
    }

    for (
        const eld::interface::InterfaceWidget& widget :
        interfaces_->list()
    ) {
        if (
            sequenceMatches(
                referencedSequenceIds,
                widget.animationId
            )
        ) {
            addUse(
                info.uses,
                AnimationUse{
                    .source = "Interface",
                    .sourceId = widget.id,
                    .sourceName = {},
                    .role = "Animation",
                    .sequenceId = *widget.animationId,
                    .viaSpotAnimationId = std::nullopt,
                    .provenance = "cache"
                }
            );
        }

        if (
            sequenceMatches(
                referencedSequenceIds,
                widget.secondaryAnimationId
            )
        ) {
            addUse(
                info.uses,
                AnimationUse{
                    .source = "Interface",
                    .sourceId = widget.id,
                    .sourceName = {},
                    .role = "Secondary animation",
                    .sequenceId = *widget.secondaryAnimationId,
                    .viaSpotAnimationId = std::nullopt,
                    .provenance = "cache"
                }
            );
        }
    }

    for (const eld::definition::ItemDefinition& item : items_->list()) {
        const eld::animation::presentation::ItemAnimationProfile profile =
            presentation_->resolveItem(item);

        for (const AnimationBinding& binding : profile.bindings) {
            addPresentationBindingUses(
                info.uses,
                referencedSequenceIds,
                *spotAnimations_,
                "Item",
                item.id,
                usableName(item.name),
                binding,
                std::nullopt
            );
        }
    }

    std::sort(
        info.sequences.begin(),
        info.sequences.end(),
        [](const AnimationSequenceReference& left,
           const AnimationSequenceReference& right) {
            return left.sequenceId < right.sequenceId;
        }
    );

    std::sort(
        info.uses.begin(),
        info.uses.end(),
        [](const AnimationUse& left, const AnimationUse& right) {
            return std::tie(
                left.source,
                left.sourceId,
                left.role,
                left.sequenceId,
                left.provenance
            ) < std::tie(
                right.source,
                right.sourceId,
                right.role,
                right.sequenceId,
                right.provenance
            );
        }
    );

    return info;
}

std::vector<std::uint16_t> AnimationInspector::listIds() const {
    return animations_->listIds();
}

}
