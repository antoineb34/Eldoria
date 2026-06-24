#pragma once

#include <optional>

#include "definition/location/LocationDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

class LocationPreviewBuilder {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::LocationDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;
};

}
