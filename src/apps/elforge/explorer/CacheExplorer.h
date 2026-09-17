#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <string>

#include <SDL3/SDL.h>

#include "cache/Cache.h"
#include "map/MapLoader.h"
#include "midi/MidiLoader.h"
#include "midi/MidiPlayer.h"

#include "animation/AnimationFrameTable.h"
#include "animation/AnimationLoader.h"
#include "animation/AnimationPlayer.h"
#include "animation/AnimationPresentationCatalog.h"
#include "animation/ModelAnimator.h"
#include "model/ModelData.h"

#include "explorer/CacheExplorerState.h"
#include "explorer/tree/CacheTreeBuilder.h"
#include "views/item/ItemView.h"

#include "explorer/tree/CacheTreePanel.h"
#include "inspection/AssetDetailsPanel.h"
#include "viewport/ViewportPanel.h"

#include "archive/Archive.h"
#include "floor/FloorLoader.h"
#include "font/FontLoader.h"
#include "identity_kit/IdentityKitLoader.h"
#include "interface/WidgetLoader.h"
#include "item/ItemLoader.h"
#include "location/LocationLoader.h"
#include "message/MessageLoader.h"
#include "message_animation/MessageAnimationLoader.h"
#include "model/ModelLoader.h"
#include "npc/NpcLoader.h"
#include "parameter/ParameterLoader.h"
#include "sequence/SequenceLoader.h"
#include "spot_animation/SpotAnimationLoader.h"
#include "sprite/SpriteLoader.h"
#include "texture/TextureLoader.h"
#include "title/TitleLoader.h"
#include "varbit/VarbitLoader.h"
#include "varp/VarpLoader.h"

#include "model/ModelSystem.h"
#include "texture/TextureSystem.h"

#include "render/model/ModelManager.h"
#include "render/texture/TextureManager.h"

namespace eld::elforge {

class CacheExplorer {
public:
  CacheExplorer();

  bool initialize();
  void shutdown();

  void handleEvent(const SDL_Event &event);

  void update();
  void renderUi();

  void prepareViewport(SDL_Renderer *renderer);

  void renderViewport(SDL_Renderer *renderer);

private:
  void handleSelectionChanged();
  void resetAnimationView();

  void startAnimationView(const std::optional<std::uint16_t> &sequenceId);

  void rebuildAnimationFrame();

  void rebuildAnimationPreviewUses();

  bool activateAnimationPreviewUse(std::size_t previewIndex);

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
      const eld::animation::presentation::AnimationBinding &binding);

  void startItemActionView(
      const eld::animation::presentation::AnimationBinding &binding);

  void appendActionEffects(
      const eld::animation::presentation::AnimationBinding &binding);

  void showItemInventoryView();

  void showItemEquippedView(ItemViewGender gender);

  void rebuildNpcActionEffect(std::size_t effectIndex);

  void updateNpcActionEffects(std::uint64_t deltaMilliseconds);

  void ensureActionTargetMarker();
  void ensureActionGrid();

  bool placeActionTargetFromViewport(float mouseX, float mouseY);

  void faceNpcTowardActionTarget();

  void renderManualNpcActionComposer();

  eld::cache::Cache cache_;
  eld::map::MapLoader mapLoader_;
  eld::midi::MidiLoader midiLoader_;
  bool explorerPanelOpen_ = true;

  eld::audio::MidiPlayer midiPlayer_;

  eld::animation::AnimationLoader animationLoader_;
  eld::animation::AnimationFrameTable animationFrameTable_;
  eld::render::AnimationPlayer animationPlayer_;
  eld::render::ModelAnimator modelAnimator_;

  enum class AnimationTargetKind { None, Npc, Item, Location, SpotAnimation };

  eld::animation::presentation::AnimationPresentationCatalog
      animationPresentationCatalog_;

  AnimationTargetKind animationTargetKind_ = AnimationTargetKind::None;

  struct NpcActionEffectState {
    eld::animation::presentation::AnimationEffectBinding binding;
    eld::spot_animation::SpotAnimationData definition;
    eld::model::ModelData sourceMesh;

    std::unique_ptr<eld::render::AnimationPlayer> player;

    std::optional<eld::render::ModelHandle> modelHandle;

    std::uint64_t elapsedMilliseconds = 0;
  };

  std::vector<NpcActionEffectState> npcActionEffects_;

  std::optional<eld::animation::presentation::AnimationBinding>
      activeNpcAction_;

  std::optional<eld::animation::presentation::AnimationBinding>
      activeItemAction_;

  enum class ItemViewMode : std::uint8_t {
    Inventory,
    MaleEquipped,
    FemaleEquipped
  };

  ItemViewMode itemViewMode_ = ItemViewMode::Inventory;

  std::optional<eld::render::ModelHandle> actionTargetHandle_;

  std::optional<eld::render::ModelHandle> actionGridHandle_;

  eld::math::Vec3 actionTargetWorld_{220.0f, 0.0f, 0.0f};

  bool showActionGrid_ = false;
  bool placeActionTargetOnClick_ = false;

  bool lockNpcFacingToActionTarget_ = true;

  float actionViewArcHeight_ = 70.0f;
  float actionViewSourceHeight_ = 60.0f;

  eld::animation::presentation::AnimationAction manualActionAction_ =
      eld::animation::presentation::AnimationAction::Attack;

  int manualActionSequenceId_ = -1;
  int manualActionSpotAnimationId_ = -1;
  bool manualActionProjectile_ = true;
  int manualActionDelayMilliseconds_ = 0;
  int manualActionDurationMilliseconds_ = 700;

  std::optional<eld::model::ModelData> animationSource_;

  std::map<std::pair<std::uint16_t, std::size_t>, eld::render::ModelHandle>
      animationHandles_;

  std::uint64_t lastAnimationUpdateMs_ = 0;

  eld::texture::TextureLoader textureLoader_;
  eld::model::ModelLoader modelLoader_;
  eld::sprite::SpriteLoader titleSpriteLoader_;
  eld::sprite::SpriteLoader mediaSpriteLoader_;
  eld::title::TitleLoader titleLoader_;
  eld::font::FontLoader titleFontLoader_;
  eld::archive::Archive definitionArchive_;
  eld::floor::FloorLoader floorLoader_;
  eld::identity_kit::IdentityKitLoader identityKitLoader_;
  eld::location::LocationLoader locationLoader_;
  eld::npc::NpcLoader npcLoader_;
  eld::item::ItemLoader itemLoader_;
  eld::sequence::SequenceLoader sequenceLoader_;
  eld::spot_animation::SpotAnimationLoader spotAnimationLoader_;
  eld::varp::VarpLoader varpLoader_;
  eld::varbit::VarbitLoader varbitLoader_;
  eld::parameter::ParameterLoader parameterLoader_;
  eld::message::MessageLoader messageLoader_;
  eld::message_animation::MessageAnimationLoader messageAnimationLoader_;
  eld::interface::WidgetLoader widgetLoader_;

  eld::render::TextureManager textureManager_;
  eld::graphics::TextureSystem textureSystem_;

  eld::render::ModelManager modelManager_;
  eld::graphics::ModelSystem modelSystem_;

  CacheExplorerState state_;
  CacheTreeBuilder treeBuilder_;

  CacheTreePanel treePanel_;
  ViewportPanel viewportPanel_;
  AssetDetailsPanel detailsPanel_;

  std::string lastSelectedKey_;
};

} // namespace eld::elforge
