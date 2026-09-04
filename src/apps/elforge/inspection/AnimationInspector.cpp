#include "animation/AnimationPlayer.h"
#include "inspection/AnimationInspector.h"

#include <algorithm>
#include <set>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>

#include "animation/AnimationAction.h"

namespace eld::elforge {

namespace {

std::uint64_t sequenceDurationMilliseconds(
    const eld::animation::AnimationFrameTable& frames,
    const eld::sequence::Sequence& sequence
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
    const eld::sequence::Sequence& sequence,
    const std::unordered_set<std::uint16_t>& animationFrameIds
) {
    SequenceArchiveUsage usage;

    for (
        const eld::sequence::SequenceFrame& frame :
        sequence.frames
    ) {
        ++usage.totalReferences;

        if (
            animationFrameIds.contains(
                frame.primaryFrameId
            )
        ) {
            ++usage.primary;
        }

        if (frame.secondaryFrameId.has_value()) {
            ++usage.totalReferences;

            if (
                animationFrameIds.contains(
                    *frame.secondaryFrameId
                )
            ) {
                ++usage.secondary;
            }
        }
    }

    return usage;
}

std::optional<std::uint16_t> cacheNpcSequenceForAction(
    const eld::npc::Npc& npc,
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
    const eld::npc::Npc& npc,
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
    const eld::spot_animation::SpotAnimationRepository& spots,
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
        const auto spot =
            spots.find(effect.spotAnimationId);

        if (
            !spot ||
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
    const eld::animation::AnimationFrameTable& frames,
    const eld::sequence::SequenceRepository& sequences,
    const eld::npc::NpcRepository& npcs,
    const eld::location::LocationRepository& locations,
    const eld::spot_animation::SpotAnimationRepository& spotAnimations,
    const eld::item::ItemRepository& items,
    const eld::interface::WidgetRepository& interfaces,
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

    std::unordered_set<std::uint16_t> animationFrameIds;
    animationFrameIds.reserve(
        info.animation.frames.size()
    );

    for (
        const eld::animation::AnimationFrame& frame :
        info.animation.frames
    ) {
        animationFrameIds.insert(
            frame.id
        );
    }

    std::unordered_set<std::uint16_t> referencedSequenceIds;

    for (const auto id : sequences_->listIds()) {
        const eld::sequence::Sequence sequence =
            sequences_->get(id);
        const SequenceArchiveUsage usage =
            sequenceArchiveUsage(
                sequence,
                animationFrameIds
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

    for (const auto id : npcs_->listIds()) {
        const eld::npc::Npc npc =
            npcs_->get(id);
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

    for (const auto id : locations_->listIds()) {
        const eld::location::Location location =
            locations_->get(id);
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

    for (const auto id : spotAnimations_->listIds()) {
        const eld::spot_animation::SpotAnimation spot =
            spotAnimations_->get(id);
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
        const eld::interface::Widget& widget :
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

    for (const auto id : items_->listIds()) {
        const eld::item::Item item =
            items_->get(id);
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
