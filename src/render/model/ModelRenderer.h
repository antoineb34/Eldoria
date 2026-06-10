#pragma once

#include <SDL3/SDL.h>

#include <unordered_map>

#include "model/ModelAsset.h"
#include "../../core/assets/texture/TextureAsset.h"

#include "../software/camera/Camera.h"

#include "RenderOptions.h"
#include "RenderMeshBuilder.h"
#include "../order/FaceOrderer.h"

namespace rf::render {

void drawModel(
    SDL_Renderer* renderer,
    const rf::model::ModelAsset& model,
    const Camera& camera,
    const RenderOptions& options,
    const ModelTransform& transform
);

}
