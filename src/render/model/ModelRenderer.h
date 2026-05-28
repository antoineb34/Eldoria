#pragma once

#include <SDL3/SDL.h>

#include <unordered_map>

#include "../../core/assets/model/ModelAsset.h"
#include "../../core/assets/texture/TextureAsset.h"

#include "../software/camera/Camera.h"
#include "../software/raster/DepthBuffer.h"

#include "RenderOptions.h"

namespace rf::render {

void drawModel(
    SDL_Renderer* renderer,
    DepthBuffer& depthBuffer,
    const rf::model::ModelAsset& model,    const Camera& camera,
    const RenderOptions& options,
    const ModelTransform& transform
);

}
