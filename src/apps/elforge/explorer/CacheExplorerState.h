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

#include "Midi.h"
#include "views/animation/AnimationViewState.h"
#include "views/midi/MidiViewState.h"

#include "render/model/ModelHandle.h"
#include "Model.h"
#include "Texture.h"
#include "Sprite.h"
#include "Image.h"
#include "Font.h"
#include "Floor.h"
#include "IdentityKit.h"
#include "Location.h"
#include "Npc.h"
#include "Item.h"
#include "Sequence.h"
#include "SpotAnimation.h"
#include "Varp.h"
#include "Varbit.h"
#include "Parameter.h"
#include "Message.h"
#include "MessageAnimation.h"
#include "Widget.h"

#include "render/camera/Camera.h"
#include "render/scene/Transform.h"

namespace eld::elforge {

enum class ViewportGizmoMode : std::uint8_t {
    Move,
    Rotate,
    Scale
};

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

    std::optional<eld::midi::Midi> activeMidi;
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

    std::optional<eld::render::ModelHandle>
        activeModelHandle;

    std::vector<PresentationRenderObject>
        presentationObjects;

    std::optional<eld::texture::Texture> activeTexture;

    std::optional<eld::sprite::Sprite> activeSprite;
    std::optional<eld::image::Image> activeImage;
    std::optional<eld::font::Font> activeFont;
    std::optional<eld::floor::Floor> activeFloor;
    std::optional<eld::identity_kit::IdentityKit>
        activeIdentityKit;

    std::optional<eld::location::Location>
        activeLocation;

    std::optional<eld::npc::Npc>
        activeNpc;

    std::optional<eld::item::Item>
        activeItem;

    std::optional<eld::sequence::Sequence>
        activeSequence;

    std::optional<eld::spot_animation::SpotAnimation>
        activeSpotAnimation;

    std::optional<eld::varp::Varp>
        activeVarp;

    std::optional<eld::varbit::Varbit>
        activeVarbit;

    std::optional<eld::parameter::Parameter>
        activeParameter;

    std::optional<eld::message::Message>
        activeMessage;

    std::optional<eld::message_animation::MessageAnimation>
        activeMessageAnimation;

    std::optional<eld::interface::Widget>
        activeInterface;

    std::string activeInterfaceDump;

    std::unordered_map<std::string, bool>
        expandedNodes;
};

}
