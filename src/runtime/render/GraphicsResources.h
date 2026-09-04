#pragma once

#include <cstdint>

#include "model/ModelHandle.h"
#include "model/ModelRegistry.h"
#include "model/ModelResolver.h"
#include "Model.h"
#include "texture/GraphicsTexture.h"
#include "texture/TextureHandle.h"
#include "texture/TextureRegistry.h"
#include "texture/TextureResolver.h"

namespace eld::render {

class GraphicsResources {
public:
    GraphicsResources(
        eld::model::ModelRepository& modelRepository,
        eld::texture::TextureRepository& textureRepository
    );

    ModelHandle resolveModel(
        std::uint16_t sourceModelId
    );

    ModelHandle resolveModel(
        const eld::model::Model& source
    );

    // Register an already-normalized backend-independent render model.
    // This is used by graphics builders (for example static map batches)
    // that are not sourced from a single cache Model.
    ModelHandle registerModel(
        RenderModel model
    );

    TextureHandle resolveTexture(
        std::uint16_t sourceTextureId
    );

    const RenderModel& getModel(
        ModelHandle handle
    ) const;

    const GraphicsTexture& getTexture(
        TextureHandle handle
    ) const;

private:
    TextureRegistry textureRegistry_;
    ModelRegistry modelRegistry_;

    TextureResolver textureResolver_;
    ModelResolver modelResolver_;
};

}
