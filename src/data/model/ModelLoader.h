#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "ModelAsset.h"
#include "ModelBuilder.h"
#include "ModelFileReader.h"

#include "../../cache/Cache.h"
#include "../texture/TextureLoader.h"

namespace rf::model {

class ModelLoader {
public:
    ModelLoader(
        const rf::cache::Cache& cache,
        rf::texture::TextureLoader& textureLoader
    );

    std::optional<ModelAsset> load(
        std::uint32_t id
    );

private:
    std::optional<std::vector<uint8_t>> getModelFile(
        std::uint32_t id
    ) const;

    std::optional<std::vector<uint8_t>> decompressPayload(
        const std::vector<uint8_t>& payload
    ) const;

private:
    const rf::cache::Cache& cache_;
    rf::texture::TextureLoader& textureLoader_;

    ModelFileReader fileReader_;
    ModelBuilder modelBuilder_;
    void loadModelTextures(ModelAsset& asset);

    std::unordered_map<
        std::uint32_t,
        ModelAsset
    > modelCache_;
};

}
