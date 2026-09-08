#pragma once

#include <cstdint>
#include <unordered_map>

#include "ModelHandle.h"
#include "ModelRegistry.h"
#include "RenderModelAssembler.h"
#include "model/ModelPipeline.h"
#include "texture/TextureResolver.h"

namespace eld::render {

class ModelResolver {
public:
    ModelResolver(
        eld::model::ModelPipeline& modelPipeline,
        TextureResolver& textureResolver,
        ModelRegistry& modelRegistry
    );

    ModelHandle resolve(
        std::uint16_t sourceModelId
    );

    ModelHandle resolve(
        const eld::model::ModelData& source
    );

private:
    eld::model::ModelPipeline& modelPipeline_;
    ModelRegistry& modelRegistry_;

    RenderModelAssembler assembler_;

    std::unordered_map<std::uint16_t, ModelHandle>
        resolvedModels_;
};

}
