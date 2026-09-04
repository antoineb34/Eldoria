#pragma once

#include <optional>

#include "repositories/IdentityKitRepository.h"
#include "Item.h"
#include "Model.h"
#include "repositories/ModelRepository.h"

namespace eld::elforge {

enum class ItemViewGender {
    Male,
    Female
};

class ItemView {
public:
    std::optional<eld::model::Model> build(
        const eld::item::Item& definition,
        const eld::model::ModelRepository& repository
    ) const;

    bool hasEquippedModel(
        const eld::item::Item& definition,
        ItemViewGender gender
    ) const;

    std::optional<eld::model::Model> buildEquipped(
        const eld::item::Item& definition,
        ItemViewGender gender,
        const eld::identity_kit::IdentityKitRepository& identityKits,
        const eld::model::ModelRepository& repository
    ) const;
};

}
