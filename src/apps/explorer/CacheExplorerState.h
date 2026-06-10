#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include "CacheSelection.h"
#include "CacheTreeNode.h"

#include "model/ModelAsset.h"
#include "../../core/assets/texture/TextureAsset.h"

#include "../../render/software/camera/Camera.h"
#include "../../render/model/RenderOptions.h"
#include "../../render/model/ModelTransform.h"

namespace rf::explorer {

struct CacheExplorerState {

    rf::render::Camera camera;
    rf::render::RenderOptions renderOptions;
    rf::render::ModelTransform modelTransform;

    int viewportX = 0;
    int viewportY = 0;
    int viewportWidth = 1;
    int viewportHeight = 1;

    CacheSelection selection;
    CacheTreeNode rootNode;
    bool debugHighlightTexturedFaces = false;

    std::optional<rf::model::ModelAsset> activeModel;
    std::optional<rf::texture::TextureAsset> activeTexture;

    std::unordered_map<std::string, bool> expandedNodes;
};

}
