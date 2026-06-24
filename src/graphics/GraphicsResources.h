#pragma once

#include <cstdint>

#include "model/ModelHandle.h"
#include "model/ModelRegistry.h"
#include "model/ModelResolver.h"
#include "model/ModelMesh.h"
#include "texture/GraphicsTexture.h"
#include "texture/TextureHandle.h"
#include "texture/TextureRegistry.h"
#include "texture/TextureResolver.h"

namespace eld::graphics {

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
        const eld::model::ModelMesh& source
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
