#include "ModelManager.h"

#include <stdexcept>
#include <utility>

namespace eld::render {

ModelHandle ModelManager::create(
    ModelResource resource
) {
    for (
        std::uint32_t i = 0;
        i < slots_.size();
        ++i
    ) {
        auto& slot = slots_[i];

        if (!slot.resource.has_value()) {
            slot.resource =
                std::move(resource);

            return {
                i,
                slot.generation
            };
        }
    }

    Slot slot;
    slot.resource =
        std::move(resource);

    slots_.push_back(
        std::move(slot)
    );

    const auto index =
        static_cast<std::uint32_t>(
            slots_.size() - 1
        );

    return {
        index,
        slots_[index].generation
    };
}


const ModelResource& ModelManager::get(
    ModelHandle handle
) const {
    if (!isValid(handle)) {
        throw std::out_of_range(
            "Invalid ModelHandle"
        );
    }

    return *slots_[handle.index].resource;
}


bool ModelManager::isValid(
    ModelHandle handle
) const {
    if (handle.index >= slots_.size()) {
        return false;
    }

    const auto& slot =
        slots_[handle.index];

    return
        slot.resource.has_value() &&
        slot.generation ==
            handle.generation;
}


void ModelManager::destroy(
    ModelHandle handle
) {
    if (!isValid(handle)) {
        throw std::out_of_range(
            "Invalid ModelHandle"
        );
    }

    auto& slot =
        slots_[handle.index];

    slot.resource.reset();
    ++slot.generation;

    if (slot.generation == 0) {
        slot.generation = 1;
    }
}

}
