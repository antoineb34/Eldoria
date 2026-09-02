#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "explorer/CacheSelection.h"
#include "explorer/tree/CacheTreeNode.h"
#include "inspection/AnimationInspector.h"
#include "export/AnimationExporter.h"
#include "views/map/MapViewState.h"

#include "midi/MidiFile.h"
#include "views/animation/AnimationViewState.h"
#include "views/midi/MidiViewState.h"

#include "graphics/model/ModelHandle.h"
#include "model/Model.h"
#include "texture/Texture.h"
#include "sprite/Sprite.h"
#include "image/Image.h"
#include "font/Font.h"
#include "definition/floor/FloorDefinition.h"
#include "definition/identity_kit/IdentityKitDefinition.h"
#include "definition/location/LocationDefinition.h"
#include "definition/npc/NpcDefinition.h"
#include "definition/item/ItemDefinition.h"
#include "definition/sequence/SequenceDefinition.h"
#include "definition/spot_animation/SpotAnimationDefinition.h"
#include "definition/varp/VarpDefinition.h"
#include "definition/varbit/VarbitDefinition.h"
#include "definition/parameter/ParameterDefinition.h"
#include "definition/message/MessageDefinition.h"
#include "definition/message_animation/MessageAnimationDefinition.h"
#include "interface/InterfaceDefinition.h"

#include "render/camera/Camera.h"
#include "render/scene/Transform.h"

namespace eld::elforge {

enum class ViewportGizmoMode : std::uint8_t {
    Move,
    Rotate,
    Scale
};

struct PresentationRenderObject {
    eld::graphics::ModelHandle model;
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
    ViewportGizmoMode viewportGizmoMode =
        ViewportGizmoMode::Rotate;

    bool showViewportGizmo = true;
    bool showEditorGrid = true;

    // Editor camera orbit state. This is independent from modelTransform:
    // the model moves through the world, while the camera moves around it.
    eld::math::Vec3 viewportCameraPivot{
        0.0f,
        0.0f,
        0.0f
    };

    float viewportCameraDistance = 650.0f;

    CacheSelection selection;
    CacheTreeNode rootNode;

    std::string assetDumpStatus;

    std::optional<AnimationInspection> activeAnimation;
    AnimationViewState animationView;
    std::string animationDumpStatus;
    std::string animationExportStatus;
    bool animationExportAllRequested = false;

    std::optional<eld::midi::MidiFile> activeMidi;
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

    std::optional<eld::model::Model> activeModel;

    std::optional<eld::graphics::ModelHandle>
        activeModelHandle;

    std::vector<PresentationRenderObject>
        presentationObjects;

    std::optional<eld::texture::Texture> activeTexture;

    std::optional<eld::sprite::Sprite> activeSprite;
    std::optional<eld::image::Image> activeImage;
    std::optional<eld::font::Font> activeFont;
    std::optional<eld::definition::FloorDefinition> activeFloor;
    std::optional<eld::definition::IdentityKitDefinition>
        activeIdentityKit;

    std::optional<eld::definition::LocationDefinition>
        activeLocation;

    std::optional<eld::definition::NpcDefinition>
        activeNpc;

    std::optional<eld::definition::ItemDefinition>
        activeItem;

    std::optional<eld::definition::SequenceDefinition>
        activeSequence;

    std::optional<eld::definition::SpotAnimationDefinition>
        activeSpotAnimation;

    std::optional<eld::definition::VarpDefinition>
        activeVarp;

    std::optional<eld::definition::VarbitDefinition>
        activeVarbit;

    std::optional<eld::definition::ParameterDefinition>
        activeParameter;

    std::optional<eld::definition::MessageDefinition>
        activeMessage;

    std::optional<eld::definition::MessageAnimationDefinition>
        activeMessageAnimation;

    std::optional<eld::interface::InterfaceWidget>
        activeInterface;

    std::string activeInterfaceDump;

    std::unordered_map<std::string, bool>
        expandedNodes;
};

}
