#pragma once

#include <optional>

#include "definition/idk/IdentityKitRepository.h"
#include "definition/item/ItemDefinition.h"
#include "model/Model.h"
#include "model/ModelRepository.h"

namespace eld::elforge {

enum class ItemPreviewGender {
    Male,
    Female
};

class ItemPreviewBuilder {
public:
    std::optional<eld::model::Model> build(
        const eld::definition::ItemDefinition& definition,
        const eld::model::ModelRepository& repository
    ) const;

    bool hasEquippedModel(
        const eld::definition::ItemDefinition& definition,
        ItemPreviewGender gender
    ) const;

    std::optional<eld::model::Model> buildEquipped(
        const eld::definition::ItemDefinition& definition,
        ItemPreviewGender gender,
        const eld::definition::IdentityKitRepository& identityKits,
        const eld::model::ModelRepository& repository
    ) const;
};

}
