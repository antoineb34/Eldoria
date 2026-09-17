#pragma once

#include <cstdint>
#include <unordered_map>

#include "model/ModelData.h"
#include "model/ModelLoader.h"
#include "render/model/ModelHandle.h"
#include "render/model/ModelManager.h"

#include "ModelBuilder.h"

namespace eld::graphics {

class ModelSystem {
public:
    ModelSystem(
        const eld::model::ModelLoader& loader,
        TextureSystem& textures,
        eld::render::ModelManager& manager);

    // Resolve/cache a RuneScape source model ID.
    eld::render::ModelHandle get(
        std::uint16_t modelId);

    // Convert a transient/modified ModelData without
    // associating it with a source model ID.
    eld::render::ModelHandle create(
        const eld::model::ModelData& source);

private:
    const eld::model::ModelLoader& loader_;
    eld::render::ModelManager& manager_;

    ModelBuilder builder_;

    std::unordered_map<
        std::uint16_t,
        eld::render::ModelHandle
    > handles_;
};

}
