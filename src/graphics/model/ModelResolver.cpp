#include "ModelResolver.h"

#include <utility>

namespace eld::graphics {

ModelResolver::ModelResolver(
    eld::model::ModelRepository& modelRepository,
    TextureResolver& textureResolver,
    ModelRegistry& modelRegistry
)
    : modelRepository_(modelRepository),
      modelRegistry_(modelRegistry),
      assembler_(textureResolver) {
}

ModelHandle ModelResolver::resolve(
    std::uint16_t sourceModelId
) {
    const auto existing =
        resolvedModels_.find(
            sourceModelId
        );

    if (existing != resolvedModels_.end()) {
        return existing->second;
    }

    const eld::model::ModelMesh source =
        modelRepository_.getMesh(
            sourceModelId
        );

    RenderModel model =
        assembler_.assemble(
            source
        );

    const ModelHandle handle =
        modelRegistry_.registerModel(
            std::move(model)
        );

    resolvedModels_.emplace(
        sourceModelId,
        handle
    );

    return handle;
}

}
