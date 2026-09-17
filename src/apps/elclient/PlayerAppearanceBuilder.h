#pragma once

#include <optional>

#include "identity_kit/IdentityKitLoader.h"
#include "model/ModelData.h"
#include "model/ModelLoader.h"

namespace eld::client {

class PlayerAppearanceBuilder {
public:
    std::optional<eld::model::ModelData>
    buildDefaultMale(
        const eld::identity_kit::IdentityKitLoader& identityKits,
        const eld::model::ModelLoader& models
    ) const;

private:
    const eld::identity_kit::IdentityKitData*
    defaultKit(
        std::uint8_t bodyPartId,
        const eld::identity_kit::IdentityKitLoader& identityKits
    ) const;

    void applyRecolors(
        eld::model::ModelData& model,
        const eld::identity_kit::IdentityKitData& kit
    ) const;

    void appendModel(
        eld::model::ModelData& destination,
        eld::model::ModelData source
    ) const;
};

}
