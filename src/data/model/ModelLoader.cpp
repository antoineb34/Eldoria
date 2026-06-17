#include "ModelLoader.h"

#include <exception>
#include <limits>

namespace eld::model {

ModelLoader::ModelLoader(
    const eld::cache::Cache& cache,
    eld::texture::TextureLoader& textureLoader
)
    : cache_(cache),
      textureLoader_(textureLoader) {
}

std::optional<ModelAsset> ModelLoader::load(
    std::uint32_t id
) {
    const auto cached =
        modelCache_.find(id);

    if (cached != modelCache_.end()) {
        return cached->second;
    }

    std::optional<std::vector<std::uint8_t>> payload =
        getModelFile(id);

    if (!payload.has_value()) {
        return std::nullopt;
    }

    std::optional<ModelFile> file =
        fileReader_.read(
            *payload
        );

    if (!file.has_value()) {
        return std::nullopt;
    }

    ModelAsset asset =
        modelBuilder_.build(
            *file
        );

    loadModelTextures(
        asset
    );

    modelCache_.emplace(
        id,
        asset
    );

    return asset;
}

void ModelLoader::loadModelTextures(
    ModelAsset& asset
) {
    for (const Face& face : asset.faces) {
        const bool isTexturedRenderType =
            face.renderType == 2 ||
            face.renderType == 3;

        if (!isTexturedRenderType) {
            continue;
        }

        const bool hasValidMapping =
            face.textureUVMappingIndex >= 0 &&
            face.textureUVMappingIndex <
                static_cast<int>(
                    asset.textureUVMappings.size()
                );

        if (!hasValidMapping) {
            continue;
        }

        const int textureId =
            static_cast<int>(
                face.color
            );

        if (
            asset.textures.contains(
                textureId
            )
        ) {
            continue;
        }

        const std::optional<
            eld::texture::TextureAsset
        > texture =
            textureLoader_.load(
                static_cast<std::uint32_t>(
                    textureId
                )
            );

        if (texture.has_value()) {
            asset.textures.emplace(
                textureId,
                *texture
            );
        }
    }
}

std::optional<std::vector<std::uint8_t>>
ModelLoader::getModelFile(
    std::uint32_t id
) const {
    if (
        id >
        std::numeric_limits<
            std::uint16_t
        >::max()
    ) {
        return std::nullopt;
    }

    try {
        eld::cache::Store models =
            cache_.open(
                eld::cache::IndexId::Models
            );

        eld::cache::File file =
            models.get(
                static_cast<std::uint16_t>(
                    id
                )
            );

        return file.getBytes();
    }
    catch (const std::exception&) {
        return std::nullopt;
    }
}

}
