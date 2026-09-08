#include "ModelResolver.h"

#include <utility>

namespace eld::render {

ModelResolver::ModelResolver(
    eld::model::ModelPipeline& modelPipeline,
    TextureResolver& textureResolver,
    ModelRegistry& modelRegistry
)
    : modelPipeline_(modelPipeline),
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

    const eld::model::ModelData source =
        modelPipeline_.data(
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

ModelHandle ModelResolver::resolve(
    const eld::model::ModelData& source
) {
    RenderModel model =
        assembler_.assemble(
            source
        );

    return modelRegistry_.registerModel(
        std::move(model)
    );
}

}
