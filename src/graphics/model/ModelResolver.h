#pragma once

#include <cstdint>
#include <unordered_map>

#include "ModelHandle.h"
#include "ModelRegistry.h"
#include "RenderModelAssembler.h"
#include "model/ModelRepository.h"
#include "texture/TextureResolver.h"

namespace eld::graphics {

class ModelResolver {
public:
    ModelResolver(
        eld::model::ModelRepository& modelRepository,
        TextureResolver& textureResolver,
        ModelRegistry& modelRegistry
    );

    ModelHandle resolve(
        std::uint16_t sourceModelId
    );

    ModelHandle resolve(
        const eld::model::ModelMesh& source
    );

private:
    eld::model::ModelRepository& modelRepository_;
    ModelRegistry& modelRegistry_;

    RenderModelAssembler assembler_;

    std::unordered_map<std::uint16_t, ModelHandle>
        resolvedModels_;
};

}
