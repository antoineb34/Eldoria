#include "ModelLoader.h"

#include <limits>

#include "binary/Compression.h"

namespace eld::model {

ModelLoader::ModelLoader(
    const cache::Cache& cache,
    texture::TextureLoader& textureLoader
)
    : cache_(cache),
    textureLoader_(textureLoader)
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
            textureLoader_.load(textureId);

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

    std::optional<eld::cache::CacheFile> file =
        cache_.readFile(
            eld::cache::CacheIndex::Model,
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
    binary::CompressionType compressionType =
        binary::detectCompression(payload);

    if (compressionType == binary::CompressionType::Unknown) {
        return payload;
    }

    if (compressionType == binary::CompressionType::Gzip) {
        std::vector<uint8_t> decompressed =
            binary::decompressGzip(payload);

        if (decompressed.empty()) {
            return std::nullopt;
        }

        return decompressed;
    }

    return std::nullopt;
}

}
