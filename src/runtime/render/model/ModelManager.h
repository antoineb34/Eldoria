#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "ModelHandle.h"
#include "ModelResource.h"

namespace eld::render {

class ModelManager {
public:
    ModelHandle create(ModelResource resource);

    const ModelResource& get(
        ModelHandle handle
    ) const;

    bool isValid(
        ModelHandle handle
    ) const;

    void destroy(
        ModelHandle handle
    );

private:
    struct Slot {
        std::optional<ModelResource> resource;
        std::uint32_t generation = 1;
    };

    std::vector<Slot> slots_;
};

}
