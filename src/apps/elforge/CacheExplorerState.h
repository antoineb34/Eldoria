#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include "CacheSelection.h"
#include "CacheTreeNode.h"

#include "model/Model.h"
#include "texture/TextureAsset.h"

#include "../../render/camera/Camera.h"
#include "../../render/viewport/RenderOptions.h"
#include "../../render/viewport/ModelTransform.h"

namespace eld::elforge {

struct CacheExplorerState {

    eld::render::Camera camera;
    eld::render::RenderOptions renderOptions;
    eld::render::ModelTransform modelTransform;

    int viewportX = 0;
    int viewportY = 0;
    int viewportWidth = 1;
    int viewportHeight = 1;

    CacheSelection selection;
    CacheTreeNode rootNode;
    bool debugHighlightTexturedFaces = false;

    std::optional<eld::model::Model> activeModel;
    std::optional<eld::texture::TextureAsset> activeTexture;

    std::unordered_map<std::string, bool> expandedNodes;
};

}
