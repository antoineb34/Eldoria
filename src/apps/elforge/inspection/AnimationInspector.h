#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "animation/Animation.h"
#include "animation/AnimationFrameIndex.h"
#include "animation/AnimationRepository.h"
#include "animation/presentation/AnimationPresentationCatalog.h"
#include "definition/item/ItemRepository.h"
#include "definition/location/LocationRepository.h"
#include "definition/npc/NpcRepository.h"
#include "definition/sequence/SequenceRepository.h"
#include "definition/spot_animation/SpotAnimationRepository.h"
#include "interface/InterfaceRepository.h"

namespace eld::elforge {

struct AnimationSequenceReference {
    std::uint16_t sequenceId = 0;
    std::size_t matchingPrimaryFrames = 0;
    std::size_t matchingSecondaryFrames = 0;
    std::size_t totalFrameReferences = 0;
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
        const eld::animation::AnimationFrameIndex& frames,
        const eld::definition::SequenceRepository& sequences,
        const eld::definition::NpcRepository& npcs,
        const eld::definition::LocationRepository& locations,
        const eld::definition::SpotAnimationRepository& spotAnimations,
        const eld::definition::ItemRepository& items,
        const eld::interface::InterfaceRepository& interfaces,
        const eld::animation::presentation::AnimationPresentationCatalog& presentation
    );

    AnimationInspection inspect(
        std::uint16_t animationId
    ) const;

    std::vector<std::uint16_t> listIds() const;

private:
    const eld::animation::AnimationRepository* animations_ = nullptr;
    const eld::animation::AnimationFrameIndex* frames_ = nullptr;
    const eld::definition::SequenceRepository* sequences_ = nullptr;
    const eld::definition::NpcRepository* npcs_ = nullptr;
    const eld::definition::LocationRepository* locations_ = nullptr;
    const eld::definition::SpotAnimationRepository* spotAnimations_ = nullptr;
    const eld::definition::ItemRepository* items_ = nullptr;
    const eld::interface::InterfaceRepository* interfaces_ = nullptr;
    const eld::animation::presentation::AnimationPresentationCatalog* presentation_ = nullptr;
};

}
