#pragma once

#include <string>

#include <SDL3/SDL.h>

#include "cache/Cache.h"

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

    eld::cache::Cache cache_;

    eld::texture::TextureRepository textureRepository_;
    eld::model::ModelRepository modelRepository_;
    eld::sprite::SpriteRepository titleSpriteRepository_;
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
