#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "explorer/CacheSelection.h"
#include "explorer/tree/CacheTreeNode.h"
#include "export/AnimationExporter.h"
#include "inspection/AnimationInspector.h"
#include "views/map/MapViewState.h"

#include "midi/MidiData.h"
#include "views/animation/AnimationViewState.h"
#include "views/midi/MidiViewState.h"

#include "floor/FloorData.h"
#include "font/FontData.h"
#include "identity_kit/IdentityKitData.h"
#include "image/ImageData.h"
#include "interface/WidgetData.h"
#include "item/ItemData.h"
#include "location/LocationData.h"
#include "message/MessageData.h"
#include "message_animation/MessageAnimationData.h"
#include "model/ModelData.h"
#include "npc/NpcData.h"
#include "parameter/ParameterData.h"
#include "render/model/ModelHandle.h"
#include "sequence/SequenceResource.h"
#include "spot_animation/SpotAnimationData.h"
#include "sprite/SpriteResource.h"
#include "texture/TextureResource.h"
#include "varbit/VarbitData.h"
#include "varp/VarpData.h"

#include "render/camera/Camera.h"
#include "render/scene/Transform.h"

namespace eld::elforge {

enum class ViewportGizmoMode : std::uint8_t { Move, Rotate, Scale };

struct PresentationRenderObject {
  eld::render::ModelHandle model;
  eld::render::Transform transform;
};

struct MapTileSelection {
  std::size_t plane = 0;
  int x = 0;
  int y = 0;
};

struct CacheExplorerState {
  eld::render::Camera camera;
  eld::render::Transform modelTransform;

  int viewportX = 0;
  int viewportY = 0;
  int viewportWidth = 1;
  int viewportHeight = 1;

  // Viewport-editor presentation state. These are tooling controls, not
  // model/cache data.
  ViewportGizmoMode viewportGizmoMode = ViewportGizmoMode::Rotate;

  bool showViewportGizmo = true;
  bool showEditorGrid = true;

  // Editor camera orbit state. This is independent from modelTransform:
  // the model moves through the world, while the camera moves around it.
  eld::math::Vec3 viewportCameraPivot{0.0f, 0.0f, 0.0f};

  float viewportCameraDistance = 650.0f;

  CacheSelection selection;
  CacheTreeNode rootNode;

  std::string assetDumpStatus;

  std::optional<AnimationInspection> activeAnimation;
  AnimationViewState animationView;
  std::string animationDumpStatus;
  std::string animationExportStatus;
  bool animationExportAllRequested = false;

  std::optional<eld::midi::MidiData> activeMidi;
  std::string midiExportStatus;
  MidiViewState midiView;

  std::optional<MapViewState> activeMap;
  std::string mapViewError;

  std::size_t mapPlane = 0;
  bool mapShowTerrain = true;
  bool mapShowLocs = true;
  float mapYaw = 0.75f;
  float mapPitch = 0.62f;
  float mapDistance = 82.0f;
  bool mapViewportDirty = true;

  std::optional<MapTileSelection> selectedMapTile;
  std::optional<std::size_t> selectedMapLocIndex;

  std::optional<eld::model::ModelData> activeModel;

  std::optional<eld::render::ModelHandle> activeModelHandle;

  std::vector<PresentationRenderObject> presentationObjects;

  std::optional<eld::texture::TextureResource> activeTexture;

  std::optional<eld::sprite::SpriteResource> activeSprite;
  std::optional<eld::image::ImageData> activeImage;
  std::optional<eld::font::FontData> activeFont;
  std::optional<eld::floor::FloorData> activeFloor;
  std::optional<eld::identity_kit::IdentityKitData> activeIdentityKit;

  std::optional<eld::location::LocationData> activeLocation;

  std::optional<eld::npc::NpcData> activeNpc;

  std::optional<eld::item::ItemData> activeItem;

  std::optional<eld::sequence::SequenceResource> activeSequence;

  std::optional<eld::spot_animation::SpotAnimationData> activeSpotAnimation;

  std::optional<eld::varp::VarpData> activeVarp;

  std::optional<eld::varbit::VarbitData> activeVarbit;

  std::optional<eld::parameter::ParameterData> activeParameter;

  std::optional<eld::message::MessageData> activeMessage;

  std::optional<eld::message_animation::MessageAnimationData>
      activeMessageAnimation;

  std::optional<eld::interface::WidgetData> activeInterface;

  std::string activeInterfaceDump;

  std::unordered_map<std::string, bool> expandedNodes;
};

} // namespace eld::elforge
