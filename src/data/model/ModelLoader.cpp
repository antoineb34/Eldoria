#include "ModelLoader.h"

#include <limits>
#include <string>
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
    ModelLoadResult result =
        loadDetailed(id);

    return result.asset;
}

ModelLoadResult ModelLoader::loadDetailed(
    std::uint32_t id
) {
    auto cached = modelCache_.find(id);

    if (cached != modelCache_.end()) {
        return {
            ModelLoadStatus::Loaded,
            "Model " + std::to_string(id) + " loaded from cache.",
            cached->second
        };
    }

    std::optional<std::vector<uint8_t>> payload =
        getModelFile(id);

    if (!payload.has_value()) {
        return {
            ModelLoadStatus::MissingCacheFile,
            "Model " + std::to_string(id) + " was not found in cache index 1.",
            std::nullopt
        };
    }

    if (payload->empty()) {
        return {
            ModelLoadStatus::EmptyPayload,
            "Model " + std::to_string(id) + " has an empty cache payload.",
            std::nullopt
        };
    }

    std::optional<std::vector<uint8_t>> decompressedPayload =
        decompressPayload(*payload);

    if (!decompressedPayload.has_value()) {
        rf::compression::CompressionType compressionType =
            rf::compression::detectCompression(*payload);

        if (compressionType == rf::compression::CompressionType::Bzip2) {
            return {
                ModelLoadStatus::UnsupportedCompression,
                "Model " + std::to_string(id) + " uses unsupported Bzip2 compression.",
                std::nullopt
            };
        }

        return {
            ModelLoadStatus::DecompressionFailed,
            "Model " + std::to_string(id) + " could not be decompressed.",
            std::nullopt
        };
    }

    if (decompressedPayload->empty()) {
        return {
            ModelLoadStatus::EmptyPayload,
            "Model " + std::to_string(id) + " decompressed to an empty payload.",
            std::nullopt
        };
    }

    std::optional<ModelFile> file =
        fileReader_.read(*decompressedPayload);

    if (!file.has_value()) {
        return {
            ModelLoadStatus::InvalidModelFile,
            "Model " + std::to_string(id) + " has an invalid model footer or layout.",
            std::nullopt
        };
    }

    ModelAsset asset =
        modelBuilder_.build(*file);

    if (asset.vertices.empty() || asset.faces.empty()) {
        return {
            ModelLoadStatus::EmptyModel,
            "Model " + std::to_string(id) + " decoded but produced no renderable geometry.",
            std::nullopt
        };
    }

    loadModelTextures(asset);

    modelCache_.emplace(id, asset);

    return {
        ModelLoadStatus::Loaded,
        "Model " + std::to_string(id) + " loaded.",
        asset
    };
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
