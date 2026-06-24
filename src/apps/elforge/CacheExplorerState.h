#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "CacheSelection.h"
#include "CacheTreeNode.h"

#include "graphics/model/ModelHandle.h"
#include "model/Model.h"
#include "texture/Texture.h"
#include "sprite/Sprite.h"
#include "image/Image.h"
#include "font/Font.h"

#include "render/camera/Camera.h"
#include "render/scene/Transform.h"

namespace eld::elforge {

struct CacheExplorerState {
    eld::render::Camera camera;
    eld::render::Transform modelTransform;

    int viewportX = 0;
    int viewportY = 0;
    int viewportWidth = 1;
    int viewportHeight = 1;

    CacheSelection selection;
    CacheTreeNode rootNode;

    std::optional<eld::model::Model> activeModel;

    std::optional<eld::graphics::ModelHandle>
        activeModelHandle;

    std::optional<eld::texture::Texture> activeTexture;

    std::optional<eld::sprite::Sprite> activeSprite;
    std::optional<eld::image::Image> activeImage;
    std::optional<eld::font::Font> activeFont;

    std::unordered_map<std::string, bool>
        expandedNodes;
};

}
