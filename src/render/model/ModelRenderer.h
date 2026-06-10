#pragma once

#include <SDL3/SDL.h>

#include <unordered_map>

#include "model/ModelAsset.h"
#include "texture/TextureAsset.h"

#include "../camera/Camera.h"

#include "RenderOptions.h"
#include "RenderMeshBuilder.h"
#include "../order/FaceOrderer.h"

namespace eld::render {

void drawModel(
    SDL_Renderer* renderer,
    const eld::model::ModelAsset& model,
    const Camera& camera,
    const RenderOptions& options,
    const ModelTransform& transform
);

}
