#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "location/LocationData.h"
#include "model/ModelLoader.h"
#include "model/ModelSystem.h"
#include "render/model/ModelHandle.h"
#include "world/Location.h"

namespace eld::graphics {

class LocationModelBuilder {
public:
    LocationModelBuilder(
        eld::model::ModelLoader& models,
        ModelSystem& modelSystem
    );

    std::optional<eld::render::ModelHandle> build(
        const eld::world::Location& location,
        const eld::location::LocationData& definition,
        std::uint8_t modelType,
        int modelRotation
    );

    std::size_t createdVariants() const;
    std::size_t missingModels() const;

private:
    struct VariantKey {
        std::uint16_t locationId = 0;
        std::uint8_t modelType = 0;
        std::uint8_t modelRotation = 0;

        bool operator==(
            const VariantKey&
        ) const = default;
    };

    struct VariantKeyHash {
        std::size_t operator()(
            const VariantKey& key
        ) const;
    };

    eld::model::ModelLoader& models_;
    ModelSystem& modelSystem_;

    std::unordered_map<
        VariantKey,
        eld::render::ModelHandle,
        VariantKeyHash
    > variants_;

    std::size_t createdVariants_ = 0;
    std::size_t missingModels_ = 0;
};

}
