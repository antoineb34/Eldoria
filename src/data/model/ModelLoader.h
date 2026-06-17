#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "ModelAsset.h"
#include "ModelBuilder.h"
#include "ModelFileReader.h"

#include "cache/Cache.h"
#include "../texture/TextureLoader.h"

namespace eld::model {

class ModelLoader {
public:
    ModelLoader(
        const eld::cache::Cache& cache,
        eld::texture::TextureLoader& textureLoader
    );

    std::optional<ModelAsset> load(
        std::uint32_t id
    );

private:
    std::optional<std::vector<std::uint8_t>>
    getModelFile(
        std::uint32_t id
    ) const;

    void loadModelTextures(
        ModelAsset& asset
    );

private:
    const eld::cache::Cache& cache_;
    eld::texture::TextureLoader& textureLoader_;

    ModelFileReader fileReader_;
    ModelBuilder modelBuilder_;

    std::unordered_map<
        std::uint32_t,
        ModelAsset
    > modelCache_;
};

}
