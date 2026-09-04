#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Animation.h"
#include "AnimationFrameTable.h"
#include "repositories/AnimationRepository.h"
#include "animation/AnimationPresentationCatalog.h"
#include "repositories/ItemRepository.h"
#include "repositories/LocationRepository.h"
#include "repositories/NpcRepository.h"
#include "repositories/SequenceRepository.h"
#include "repositories/SpotAnimationRepository.h"
#include "repositories/WidgetRepository.h"

namespace eld::elforge {

struct AnimationSequenceReference {
    std::uint16_t sequenceId = 0;
    std::size_t matchingPrimaryFrames = 0;
    std::size_t matchingSecondaryFrames = 0;
    std::size_t totalFrameReferences = 0;
    std::uint64_t durationMilliseconds = 0;
};

struct AnimationUse {
    std::string source;
    std::uint16_t sourceId = 0;
    std::string sourceName;
    std::string role;
    std::uint16_t sequenceId = 0;
    std::optional<std::uint16_t> viaSpotAnimationId;
    std::string provenance;
};

struct AnimationInspection {
    eld::animation::Animation animation;
    std::vector<AnimationSequenceReference> sequences;
    std::vector<AnimationUse> uses;
};

class AnimationInspector {
public:
    AnimationInspector(
        const eld::animation::AnimationRepository& animations,
        const eld::animation::AnimationFrameTable& frames,
        const eld::sequence::SequenceRepository& sequences,
        const eld::npc::NpcRepository& npcs,
        const eld::location::LocationRepository& locations,
        const eld::spot_animation::SpotAnimationRepository& spotAnimations,
        const eld::item::ItemRepository& items,
        const eld::interface::WidgetRepository& interfaces,
        const eld::animation::presentation::AnimationPresentationCatalog& presentation
    );

    AnimationInspection inspect(
        std::uint16_t animationId
    ) const;

    std::vector<std::uint16_t> listIds() const;

private:
    const eld::animation::AnimationRepository* animations_ = nullptr;
    const eld::animation::AnimationFrameTable* frames_ = nullptr;
    const eld::sequence::SequenceRepository* sequences_ = nullptr;
    const eld::npc::NpcRepository* npcs_ = nullptr;
    const eld::location::LocationRepository* locations_ = nullptr;
    const eld::spot_animation::SpotAnimationRepository* spotAnimations_ = nullptr;
    const eld::item::ItemRepository* items_ = nullptr;
    const eld::interface::WidgetRepository* interfaces_ = nullptr;
    const eld::animation::presentation::AnimationPresentationCatalog* presentation_ = nullptr;
};

}
