#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "animation/AnimationData.h"
#include "animation/AnimationLoader.h"
#include "animation/AnimationPresentationCatalog.h"
#include "repositories/ItemRepository.h"
#include "repositories/LocationRepository.h"
#include "repositories/NpcRepository.h"
#include "sequence/SequenceLoader.h"
#include "spot_animation/SpotAnimationLoader.h"
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
    eld::animation::AnimationData animation;
    std::vector<AnimationSequenceReference> sequences;
    std::vector<AnimationUse> uses;
};

class AnimationInspector {
public:
    AnimationInspector(
        const eld::animation::AnimationLoader& animations,
        const eld::sequence::SequenceLoader& sequences,
        const eld::npc::NpcRepository& npcs,
        const eld::location::LocationRepository& locations,
        const eld::spot_animation::SpotAnimationLoader& spotAnimations,
        const eld::item::ItemRepository& items,
        const eld::interface::WidgetRepository& interfaces,
        const eld::animation::presentation::AnimationPresentationCatalog& presentation
    );

    AnimationInspection inspect(
        std::uint16_t animationId
    ) const;

    std::vector<std::uint16_t> listIds() const;

private:
    const eld::animation::AnimationLoader* animations_ = nullptr;
    const eld::sequence::SequenceLoader* sequences_ = nullptr;
    const eld::npc::NpcRepository* npcs_ = nullptr;
    const eld::location::LocationRepository* locations_ = nullptr;
    const eld::spot_animation::SpotAnimationLoader* spotAnimations_ = nullptr;
    const eld::item::ItemRepository* items_ = nullptr;
    const eld::interface::WidgetRepository* interfaces_ = nullptr;
    const eld::animation::presentation::AnimationPresentationCatalog* presentation_ = nullptr;
};

}
