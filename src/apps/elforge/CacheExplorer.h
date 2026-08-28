#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <utility>

#include <string>

#include <SDL3/SDL.h>

#include "cache/Cache.h"

#include "animation/AnimationFrameIndex.h"
#include "animation/AnimationRepository.h"
#include "animation/AnimationPlayer.h"
#include "animation/ModelAnimator.h"
#include "model/ModelMesh.h"

#include "CacheExplorerState.h"
#include "CacheTreeBuilder.h"

#include "panels/CacheInspectorPanel.h"
#include "panels/CacheTreePanel.h"
#include "panels/CacheViewportPanel.h"

#include "model/ModelRepository.h"
#include "texture/TextureRepository.h"
#include "sprite/SpriteRepository.h"
#include "image/JpegRepository.h"
#include "font/FontRepository.h"
#include "definition/DefinitionRepository.h"
#include "definition/floor/FloorRepository.h"
#include "definition/idk/IdentityKitRepository.h"
#include "definition/location/LocationRepository.h"
#include "definition/npc/NpcRepository.h"
#include "definition/item/ItemRepository.h"
#include "definition/sequence/SequenceRepository.h"
#include "definition/spot_animation/SpotAnimationRepository.h"
#include "definition/varp/VarpRepository.h"
#include "definition/varbit/VarbitRepository.h"
#include "definition/parameter/ParameterRepository.h"
#include "definition/message/MessageRepository.h"
#include "definition/message_animation/MessageAnimationRepository.h"
#include "interface/InterfaceRepository.h"

#include "graphics/GraphicsResources.h"

namespace eld::elforge {

class CacheExplorer {
public:
    CacheExplorer();

    bool initialize();

    void handleEvent(
        const SDL_Event& event
    );

    void update();
    void renderUi();

    void renderViewport(
        SDL_Renderer* renderer
    );

private:
    bool hasAlphaFaces(
        const eld::model::ModelMesh& model
    ) const;

    void handleSelectionChanged();
    void findNextAlphaModel();

    // ELFORGE_NPC_ANIMATION_PREVIEW_V1
    void resetNpcAnimationPreview();

    void startNpcAnimationPreview(
        const std::optional<std::uint16_t>& sequenceId
    );

    void rebuildNpcAnimationFrame();
    void renderNpcAnimationControls();

    eld::cache::Cache cache_;

    eld::animation::AnimationRepository animationRepository_;
    eld::animation::AnimationFrameIndex animationFrameIndex_;

    eld::graphics::AnimationPlayer animationPlayer_;
    eld::graphics::ModelAnimator modelAnimator_;

    std::optional<eld::model::ModelMesh>
        npcAnimationSource_;

    std::map<
        std::pair<std::uint16_t, std::size_t>,
        eld::graphics::ModelHandle
    > npcAnimationHandles_;

    std::uint64_t lastAnimationUpdateMs_ = 0;


    eld::texture::TextureRepository textureRepository_;
    eld::model::ModelRepository modelRepository_;
    eld::sprite::SpriteRepository titleSpriteRepository_;
    eld::sprite::SpriteRepository mediaSpriteRepository_;
    eld::image::JpegRepository titleJpegRepository_;
    eld::font::FontRepository titleFontRepository_;
    eld::definition::DefinitionRepository definitionRepository_;
    eld::definition::FloorRepository floorRepository_;
    eld::definition::IdentityKitRepository identityKitRepository_;
    eld::definition::LocationRepository locationRepository_;
    eld::definition::NpcRepository npcRepository_;
    eld::definition::ItemRepository itemRepository_;
    eld::definition::SequenceRepository sequenceRepository_;
    eld::definition::SpotAnimationRepository spotAnimationRepository_;
    eld::definition::VarpRepository varpRepository_;
    eld::definition::VarbitRepository varbitRepository_;
    eld::definition::ParameterRepository parameterRepository_;
    eld::definition::MessageRepository messageRepository_;
    eld::definition::MessageAnimationRepository messageAnimationRepository_;
    eld::interface::InterfaceRepository interfaceRepository_;

    eld::graphics::GraphicsResources graphicsResources_;
    std::optional<eld::sprite::Sprite> activeSprite;

    CacheExplorerState state_;
    CacheTreeBuilder treeBuilder_;

    CacheTreePanel treePanel_;
    CacheViewportPanel viewportPanel_;
    CacheInspectorPanel inspectorPanel_;

    std::string lastSelectedKey_;
};

}
