#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include "CacheSelection.h"
#include "CacheTreeNode.h"

#include "model/ModelAsset.h"
#include "../../../../core/assets/texture/TextureAsset.h"

#include "../../../../render/software/camera/Camera.h"
#include "../../../../render/model/RenderOptions.h"
#include "../../../../render/model/ModelTransform.h"
#include "cache/CacheFileDetails.h"

namespace eldoria::apps::elforge {

struct CacheState {
    rf::render::Camera modelViewportCamera;
    rf::render::RenderOptions modelViewportRenderOptions;
    rf::render::ModelTransform modelViewportTransform;

    int modelViewportX = 0;
    int modelViewportY = 0;
    int modelViewportWidth = 1;
    int modelViewportHeight = 1;

    CacheSelection selection;
    CacheTreeNode rootNode;
    bool modelViewportHighlightTexturedFaces = false;

    std::optional<rf::model::ModelAsset> selectedModel;
    std::optional<std::string> selectedModelLoadError;
    std::optional<rf::texture::TextureAsset> selectedModelTexture;
    std::optional<rf::cache::CacheFileDetails> selectedCacheFileDetails;

    std::unordered_map<std::string, bool> expandedNodes;
};

}
