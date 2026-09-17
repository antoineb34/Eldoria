#include "ModelSystem.h"

#include <utility>

namespace eld::graphics {

ModelSystem::ModelSystem(
    const eld::model::ModelLoader& loader,
    TextureSystem& textures,
    eld::render::ModelManager& manager
)
    : loader_(loader),
      manager_(manager),
      builder_(textures) {
}


eld::render::ModelHandle ModelSystem::get(
    std::uint16_t modelId
) {
    const auto existing =
        handles_.find(modelId);

    if (existing != handles_.end()) {
        if (manager_.isValid(existing->second)) {
            return existing->second;
        }

        handles_.erase(existing);
    }

    eld::render::ModelResource resource =
        builder_.build(
            loader_.get(modelId)
        );

    eld::render::ModelHandle handle =
        manager_.create(
            std::move(resource)
        );

    handles_.emplace(
        modelId,
        handle
    );

    return handle;
}


eld::render::ModelHandle ModelSystem::create(
    const eld::model::ModelData& source
) {
    return manager_.create(
        builder_.build(source)
    );
}

}
