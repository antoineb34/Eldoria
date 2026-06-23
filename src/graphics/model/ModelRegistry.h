#pragma once

#include <cstddef>
#include <vector>

#include "ModelHandle.h"
#include "RenderModel.h"

namespace eld::graphics {

class ModelRegistry {
public:
    ModelHandle registerModel(
        RenderModel model
    );

    const RenderModel& get(
        ModelHandle handle
    ) const;

    bool contains(
        ModelHandle handle
    ) const;

    std::size_t count() const;

private:
    std::vector<RenderModel> models_;
};

}
