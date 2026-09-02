#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>
#include <memory>
#include <optional>
#include <utility>

#include <string>

#include <SDL3/SDL.h>

#include "cache/Cache.h"
#include "map/MapLoader.h"
#include "midi/MidiRepository.h"
#include "midi/MidiPlayer.h"

#include "animation/AnimationFrameIndex.h"
#include "animation/AnimationRepository.h"
#include "animation/AnimationPlayer.h"
#include "animation/ModelAnimator.h"
#include "animation/presentation/AnimationPresentationCatalog.h"
#include "model/ModelMesh.h"

#include "explorer/CacheExplorerState.h"
#include "explorer/tree/CacheTreeBuilder.h"
#include "views/item/ItemView.h"

#include "inspection/AssetDetailsPanel.h"
#include "explorer/tree/CacheTreePanel.h"
#include "viewport/ViewportPanel.h"

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
    void shutdown();

    void handleEvent(
        const SDL_Event& event
    );

    void update();
    void renderUi();

    void prepareViewport(
        SDL_Renderer* renderer
    );

    void renderViewport(
        SDL_Renderer* renderer
    );

private:
    void handleSelectionChanged();
    void resetAnimationView();

    void startAnimationView(
        const std::optional<std::uint16_t>& sequenceId
    );

    void rebuildAnimationFrame();

    void rebuildAnimationPreviewUses();

    bool activateAnimationPreviewUse(
        std::size_t previewIndex
    );

    void selectNextNpcWithProjectile();

    void selectNextWearableItem();

    void renderNpcAnimationControls();
    void renderItemAnimationControls();
    void renderLocationAnimationControls();
    void renderSpotAnimationControls();
    void renderAnimationControls();
    void renderAnimationPlayerHud();
    void clearNpcActionView();

    void startNpcActionView(
        const eld::animation::presentation::AnimationBinding& binding
    );

    void startItemActionView(
        const eld::animation::presentation::AnimationBinding& binding
    );

    void appendActionEffects(
        const eld::animation::presentation::AnimationBinding& binding
    );

    void showItemInventoryView();

    void showItemEquippedView(
        ItemViewGender gender
    );

    void rebuildNpcActionEffect(
        std::size_t effectIndex
    );

    void updateNpcActionEffects(
        std::uint64_t deltaMilliseconds
    );

    void ensureActionTargetMarker();
    void ensureActionGrid();

    bool placeActionTargetFromViewport(
        float mouseX,
        float mouseY
    );

    void faceNpcTowardActionTarget();

    void renderManualNpcActionComposer();

    eld::cache::Cache cache_;
    eld::map::MapLoader mapLoader_;
    eld::midi::MidiRepository midiRepository_;
    bool explorerPanelOpen_ = true;

    eld::audio::MidiPlayer midiPlayer_;

    eld::animation::AnimationRepository animationRepository_;
    eld::animation::AnimationFrameIndex animationFrameIndex_;

    eld::graphics::AnimationPlayer animationPlayer_;
    eld::graphics::ModelAnimator modelAnimator_;

    enum class AnimationTargetKind {
        None,
        Npc,
        Item,
        Location,
        SpotAnimation
    };

    eld::animation::presentation::AnimationPresentationCatalog
        animationPresentationCatalog_;

    AnimationTargetKind animationTargetKind_ =
        AnimationTargetKind::None;


    struct NpcActionEffectState {
        eld::animation::presentation::AnimationEffectBinding binding;
        eld::definition::SpotAnimationDefinition definition;
        eld::model::ModelMesh sourceMesh;

        std::unique_ptr<eld::graphics::AnimationPlayer>
            player;

        std::optional<eld::graphics::ModelHandle>
            modelHandle;

        std::uint64_t elapsedMilliseconds = 0;
    };

    std::vector<NpcActionEffectState>
        npcActionEffects_;

    std::optional<eld::animation::presentation::AnimationBinding>
        activeNpcAction_;

    std::optional<eld::animation::presentation::AnimationBinding>
        activeItemAction_;

    enum class ItemViewMode : std::uint8_t {
        Inventory,
        MaleEquipped,
        FemaleEquipped
    };

    ItemViewMode itemViewMode_ =
        ItemViewMode::Inventory;

    std::optional<eld::graphics::ModelHandle>
        actionTargetHandle_;

    std::optional<eld::graphics::ModelHandle>
        actionGridHandle_;

    eld::math::Vec3 actionTargetWorld_{
        220.0f,
        0.0f,
        0.0f
    };

    bool showActionGrid_ = false;
    bool placeActionTargetOnClick_ = false;

    bool lockNpcFacingToActionTarget_ = true;

    float actionViewArcHeight_ = 70.0f;
    float actionViewSourceHeight_ = 60.0f;

    eld::animation::presentation::AnimationAction
        manualActionAction_ =
            eld::animation::presentation::AnimationAction::Attack;

    int manualActionSequenceId_ = -1;
    int manualActionSpotAnimationId_ = -1;
    bool manualActionProjectile_ = true;
    int manualActionDelayMilliseconds_ = 0;
    int manualActionDurationMilliseconds_ = 700;

    std::optional<eld::model::ModelMesh>
        animationSource_;

    std::map<
        std::pair<std::uint16_t, std::size_t>,
        eld::graphics::ModelHandle
    > animationHandles_;

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

    CacheExplorerState state_;
    CacheTreeBuilder treeBuilder_;

    CacheTreePanel treePanel_;
    ViewportPanel viewportPanel_;
    AssetDetailsPanel detailsPanel_;

    std::string lastSelectedKey_;
};

}
