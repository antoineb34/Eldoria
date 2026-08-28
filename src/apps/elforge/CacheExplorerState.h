#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "CacheSelection.h"
#include "CacheTreeNode.h"

#include "graphics/model/ModelHandle.h"
#include "model/Model.h"
#include "texture/Texture.h"
#include "sprite/Sprite.h"
#include "image/Image.h"
#include "font/Font.h"
#include "definition/floor/FloorDefinition.h"
#include "definition/idk/IdentityKitDefinition.h"
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

// ELFORGE_EDITOR_VIEWPORT_V1
enum class ViewportGizmoMode : std::uint8_t {
    Move,
    Rotate,
    Scale
};

// ELFORGE_COMPOSITE_ACTION_PREVIEW_V1
struct PresentationRenderObject {
    eld::graphics::ModelHandle model;
    eld::render::Transform transform;
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

    // ELFORGE_CAMERA_NAVIGATION_V1
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
