#pragma once

#include <optional>

#include "repositories/IdentityKitRepository.h"
#include "Item.h"
#include "model/ModelData.h"
#include "model/ModelPipeline.h"

namespace eld::elforge {

enum class ItemViewGender {
    Male,
    Female
};

class ItemView {
public:
    std::optional<eld::model::ModelData> build(
        const eld::item::Item& definition,
        const eld::model::ModelPipeline& repository
    ) const;

    bool hasEquippedModel(
        const eld::item::Item& definition,
        ItemViewGender gender
    ) const;

    std::optional<eld::model::ModelData> buildEquipped(
        const eld::item::Item& definition,
        ItemViewGender gender,
        const eld::identity_kit::IdentityKitRepository& identityKits,
        const eld::model::ModelPipeline& repository
    ) const;
};

}
