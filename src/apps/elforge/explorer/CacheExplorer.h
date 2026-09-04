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
#include "map/MapRepository.h"
#include "repositories/MidiRepository.h"
#include "midi/MidiPlayer.h"

#include "repositories/AnimationRepository.h"
#include "AnimationFrameTable.h"
#include "animation/AnimationPlayer.h"
#include "animation/ModelAnimator.h"
#include "animation/AnimationPresentationCatalog.h"
#include "Model.h"

#include "explorer/CacheExplorerState.h"
#include "explorer/tree/CacheTreeBuilder.h"
#include "views/item/ItemView.h"

#include "inspection/AssetDetailsPanel.h"
#include "explorer/tree/CacheTreePanel.h"
#include "viewport/ViewportPanel.h"

#include "repositories/ModelRepository.h"
#include "repositories/TextureRepository.h"
#include "repositories/SpriteRepository.h"
#include "repositories/ImageRepository.h"
#include "repositories/FontRepository.h"
#include "archive/Archive.h"
#include "repositories/FloorRepository.h"
#include "repositories/IdentityKitRepository.h"
#include "repositories/LocationRepository.h"
#include "repositories/NpcRepository.h"
#include "repositories/ItemRepository.h"
#include "repositories/SequenceRepository.h"
#include "repositories/SpotAnimationRepository.h"
#include "repositories/VarpRepository.h"
#include "repositories/VarbitRepository.h"
#include "repositories/ParameterRepository.h"
#include "repositories/MessageRepository.h"
#include "repositories/MessageAnimationRepository.h"
#include "repositories/WidgetRepository.h"

#include "render/GraphicsResources.h"

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
    eld::map::MapRepository mapRepository_;
    eld::midi::MidiRepository midiRepository_;
    bool explorerPanelOpen_ = true;

    eld::audio::MidiPlayer midiPlayer_;

    eld::animation::AnimationRepository animationRepository_;
    eld::animation::AnimationFrameTable animationFrameTable_;

    eld::render::AnimationPlayer animationPlayer_;
    eld::render::ModelAnimator modelAnimator_;

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
        eld::spot_animation::SpotAnimation definition;
        eld::model::Model sourceMesh;

        std::unique_ptr<eld::render::AnimationPlayer>
            player;

        std::optional<eld::render::ModelHandle>
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

    std::optional<eld::render::ModelHandle>
        actionTargetHandle_;

    std::optional<eld::render::ModelHandle>
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

    std::optional<eld::model::Model>
        animationSource_;

    std::map<
        std::pair<std::uint16_t, std::size_t>,
        eld::render::ModelHandle
    > animationHandles_;

    std::uint64_t lastAnimationUpdateMs_ = 0;


    eld::texture::TextureRepository textureRepository_;
    eld::model::ModelRepository modelRepository_;
    eld::sprite::SpriteRepository titleSpriteRepository_;
    eld::sprite::SpriteRepository mediaSpriteRepository_;
    eld::image::ImageRepository titleImageRepository_;
    eld::font::FontRepository titleFontRepository_;
    eld::archive::Archive definitionArchive_;
    eld::floor::FloorRepository floorRepository_;
    eld::identity_kit::IdentityKitRepository identityKitRepository_;
    eld::location::LocationRepository locationRepository_;
    eld::npc::NpcRepository npcRepository_;
    eld::item::ItemRepository itemRepository_;
    eld::sequence::SequenceRepository sequenceRepository_;
    eld::spot_animation::SpotAnimationRepository spotAnimationRepository_;
    eld::varp::VarpRepository varpRepository_;
    eld::varbit::VarbitRepository varbitRepository_;
    eld::parameter::ParameterRepository parameterRepository_;
    eld::message::MessageRepository messageRepository_;
    eld::message_animation::MessageAnimationRepository messageAnimationRepository_;
    eld::interface::WidgetRepository widgetRepository_;

    eld::render::GraphicsResources graphicsResources_;

    CacheExplorerState state_;
    CacheTreeBuilder treeBuilder_;

    CacheTreePanel treePanel_;
    ViewportPanel viewportPanel_;
    AssetDetailsPanel detailsPanel_;

    std::string lastSelectedKey_;
};

}
