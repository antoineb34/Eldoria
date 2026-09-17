#pragma once

#include <optional>

#include "identity_kit/IdentityKitData.h"
#include "model/ModelData.h"
#include "model/ModelLoader.h"

namespace eld::elforge {

class IdentityKitView {
public:
  std::optional<eld::model::ModelData>
  build(const eld::identity_kit::IdentityKitData &definition,
        const eld::model::ModelLoader &repository) const;
};

} // namespace eld::elforge
