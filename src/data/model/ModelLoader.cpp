#include "ModelLoader.h"

#include <limits>
#include <utility>

#include "compression/Compression.h"

namespace rf::model {

ModelLoader::ModelLoader(
    const rf::cache::Cache& cache,
    TextureLoaderCallback textureLoader
)
    : cache_(cache),
      textureLoader_(std::move(textureLoader))
{
}

std::optional<ModelAsset> ModelLoader::load(
    std::uint32_t id
) {
    auto cached = modelCache_.find(id);

    if (cached != modelCache_.end()) {
        return cached->second;
    }

    std::optional<std::vector<uint8_t>> payload =
        getModelFile(id);

    if (!payload.has_value()) {
        return std::nullopt;
    }

    std::optional<std::vector<uint8_t>> decompressedPayload =
        decompressPayload(*payload);

    if (!decompressedPayload.has_value()) {
        return std::nullopt;
    }

    std::optional<ModelFile> file =
        fileReader_.read(*decompressedPayload);

    if (!file.has_value()) {
        return std::nullopt;
    }

    ModelAsset asset =
        modelBuilder_.build(*file);

    loadModelTextures(asset);

    modelCache_.emplace(id, asset);

    return asset;
}

void ModelLoader::loadModelTextures(
    ModelAsset& asset
) {
    for (const Face& face : asset.faces) {
        bool isTexturedRenderType =
            face.renderType == 2 ||
            face.renderType == 3;

        if (!isTexturedRenderType) {
            continue;
        }

        bool hasValidMapping =
            face.textureUVMappingIndex >= 0 &&
            face.textureUVMappingIndex <
                static_cast<int>(
                    asset.textureUVMappings.size()
                );

        if (!hasValidMapping) {
            continue;
        }

        int textureId =
            static_cast<int>(
                face.color
            );

        if (asset.textures.contains(textureId)) {
            continue;
        }

        auto texture =
            textureLoader_(static_cast<std::uint32_t>(textureId));

        if (texture.has_value()) {
            asset.textures[textureId] =
                *texture;
        }
    }
}

std::optional<std::vector<uint8_t>> ModelLoader::getModelFile(
    std::uint32_t id
) const {
    if (id > static_cast<std::uint32_t>(std::numeric_limits<int>::max())) {
        return std::nullopt;
    }

    std::optional<rf::cache::CacheFile> file =
        cache_.readFile(
            rf::cache::CacheIndex::Model,
            static_cast<int>(id)
        );

    if (!file.has_value()) {
        return std::nullopt;
    }

    return file->payload;
}

std::optional<std::vector<uint8_t>> ModelLoader::decompressPayload(
    const std::vector<uint8_t>& payload
) const {
    rf::compression::CompressionType compressionType =
        rf::compression::detectCompression(payload);

    if (compressionType == rf::compression::CompressionType::Unknown) {
        return payload;
    }

    if (compressionType == rf::compression::CompressionType::Gzip) {
        std::vector<uint8_t> decompressed =
            rf::compression::decompressGzip(payload);

        if (decompressed.empty()) {
            return std::nullopt;
        }

        return decompressed;
    }

    return std::nullopt;
}

}
