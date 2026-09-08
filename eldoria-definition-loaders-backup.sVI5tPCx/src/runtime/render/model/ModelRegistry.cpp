#include "ModelRegistry.h"

#include <cstdint>
#include <utility>

namespace eld::render {

ModelHandle ModelRegistry::registerModel(
    RenderModel model
) {
    const ModelHandle handle{
        static_cast<std::uint32_t>(
            models_.size()
        )
    };

    models_.push_back(
        std::move(model)
    );

    return handle;
}

const RenderModel& ModelRegistry::get(
    ModelHandle handle
) const {
    return models_.at(
        handle.value
    );
}

bool ModelRegistry::contains(
    ModelHandle handle
) const {
    return handle.value < models_.size();
}

std::size_t ModelRegistry::count() const {
    return models_.size();
}

}
