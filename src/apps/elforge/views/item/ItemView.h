#pragma once

#include <optional>

#include "identity_kit/IdentityKitLoader.h"
#include "item/ItemData.h"
#include "model/ModelData.h"
#include "model/ModelLoader.h"

namespace eld::elforge {

enum class ItemViewGender { Male, Female };

class ItemView {
public:
  std::optional<eld::model::ModelData>
  build(const eld::item::ItemData &definition,
        const eld::model::ModelLoader &repository) const;

  bool hasEquippedModel(const eld::item::ItemData &definition,
                        ItemViewGender gender) const;

  std::optional<eld::model::ModelData>
  buildEquipped(const eld::item::ItemData &definition, ItemViewGender gender,
                const eld::identity_kit::IdentityKitLoader &identityKits,
                const eld::model::ModelLoader &repository) const;
};

} // namespace eld::elforge
