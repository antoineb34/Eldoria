#include "GraphicsResources.h"

namespace eld::graphics {

GraphicsResources::GraphicsResources(
    eld::model::ModelRepository& modelRepository,
    eld::texture::TextureRepository& textureRepository
)
    : textureResolver_(
          textureRepository,
          textureRegistry_
      ),
      modelResolver_(
          modelRepository,
          textureResolver_,
          modelRegistry_
      ) {
}

ModelHandle GraphicsResources::resolveModel(
    std::uint16_t sourceModelId
) {
    return modelResolver_.resolve(
        sourceModelId
    );
}

ModelHandle GraphicsResources::resolveModel(
    const eld::model::ModelMesh& source
) {
    return modelResolver_.resolve(
        source
    );
}

TextureHandle GraphicsResources::resolveTexture(
    std::uint16_t sourceTextureId
) {
    return textureResolver_.resolve(
        sourceTextureId
    );
}

const RenderModel& GraphicsResources::getModel(
    ModelHandle handle
) const {
    return modelRegistry_.get(handle);
}

const GraphicsTexture& GraphicsResources::getTexture(
    TextureHandle handle
) const {
    return textureRegistry_.get(handle);
}

}
