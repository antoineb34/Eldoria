#include "GraphicsResources.h"

#include <utility>

namespace eld::render {

GraphicsResources::GraphicsResources(
    eld::model::ModelLoader& modelLoader,
    eld::texture::TextureLoader& textureLoader
)
    : textureResolver_(
          textureLoader,
          textureRegistry_
      ),
      modelResolver_(
          modelLoader,
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
    const eld::model::ModelData& source
) {
    return modelResolver_.resolve(
        source
    );
}

ModelHandle GraphicsResources::registerModel(
    RenderModel model
) {
    return modelRegistry_.registerModel(
        std::move(model)
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
