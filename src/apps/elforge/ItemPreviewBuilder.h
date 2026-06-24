#pragma once

#include <optional>

#include "definition/item/ItemDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class ItemPreviewBuilder {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::ItemDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;
};

}
