#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <unordered_map>
#include <vector>

#include "ModelAsset.h"
#include "ModelBuilder.h"
#include "ModelFileReader.h"

#include "cache/Cache.h"

namespace rf::model {

using TextureLoaderCallback =
    std::function<std::optional<rf::texture::TextureAsset>(std::uint32_t)>;

class ModelLoader {
public:
    ModelLoader(
        const rf::cache::Cache& cache,
        TextureLoaderCallback textureLoader
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
    TextureLoaderCallback textureLoader_;

    ModelFileReader fileReader_;
    ModelBuilder modelBuilder_;
    void loadModelTextures(ModelAsset& asset);

    std::unordered_map<
        std::uint32_t,
        ModelAsset
    > modelCache_;
};

}
