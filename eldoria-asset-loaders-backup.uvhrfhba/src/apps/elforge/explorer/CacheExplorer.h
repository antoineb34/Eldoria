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
#include "repositories/MapRepository.h"
#include "repositories/MidiRepository.h"
#include "midi/MidiPlayer.h"

#include "animation/AnimationPipeline.h"
#include "animation/AnimationFrameTable.h"
#include "animation/AnimationPlayer.h"
#include "animation/ModelAnimator.h"
#include "animation/AnimationPresentationCatalog.h"
#include "model/ModelData.h"

#include "explorer/CacheExplorerState.h"
#include "explorer/tree/CacheTreeBuilder.h"
#include "views/item/ItemView.h"

#include "inspection/AssetDetailsPanel.h"
#include "explorer/tree/CacheTreePanel.h"
#include "viewport/ViewportPanel.h"

#include "model/ModelPipeline.h"
#include "texture/TexturePipeline.h"
#include "sprite/SpritePipeline.h"
#include "title/TitlePipeline.h"
#include "repositories/FontRepository.h"
#include "archive/Archive.h"
#include "repositories/FloorRepository.h"
#include "repositories/IdentityKitRepository.h"
#include "repositories/LocationRepository.h"
#include "repositories/NpcRepository.h"
#include "repositories/ItemRepository.h"
#include "sequence/SequencePipeline.h"
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

    eld::animation::AnimationPipeline animationPipeline_;
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
        eld::model::ModelData sourceMesh;

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

    std::optional<eld::model::ModelData>
        animationSource_;

    std::map<
        std::pair<std::uint16_t, std::size_t>,
        eld::render::ModelHandle
    > animationHandles_;

    std::uint64_t lastAnimationUpdateMs_ = 0;


    eld::texture::TexturePipeline texturePipeline_;
    eld::model::ModelPipeline modelPipeline_;
    eld::sprite::SpritePipeline titleSpritePipeline_;
    eld::sprite::SpritePipeline mediaSpritePipeline_;
    eld::title::TitlePipeline titlePipeline_;
    eld::font::FontRepository titleFontRepository_;
    eld::archive::Archive definitionArchive_;
    eld::floor::FloorRepository floorRepository_;
    eld::identity_kit::IdentityKitRepository identityKitRepository_;
    eld::location::LocationRepository locationRepository_;
    eld::npc::NpcRepository npcRepository_;
    eld::item::ItemRepository itemRepository_;
    eld::sequence::SequencePipeline sequencePipeline_;
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
