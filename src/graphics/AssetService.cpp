#include "AssetService.h"

#include <exception>
#include <utility>

namespace eld::resource {

AssetService::AssetService(
    eld::model::ModelRepository& models,
    eld::texture::TextureRepository& textures
)
    : models_(models),
      textures_(textures) {
}

const eld::model::Model&
AssetService::getModel(
    std::uint16_t id
) {
    const auto loaded =
        loadedModels_.find(id);

    if (
        loaded !=
        loadedModels_.end()
    ) {
        return loaded->second;
    }

    auto inserted =
        loadedModels_.emplace(
            id,
            models_.get(id)
        );

    return inserted.first->second;
}

const eld::texture::Texture&
AssetService::getTexture(
    std::uint16_t id
) {
    const auto loaded =
        loadedTextures_.find(id);

    if (
        loaded !=
        loadedTextures_.end()
    ) {
        return loaded->second;
    }

    auto inserted =
        loadedTextures_.emplace(
            id,
            textures_.get(id)
        );

    return inserted.first->second;
}

const eld::render::RenderModel&
AssetService::getRenderModel(
    std::uint16_t id
) {
    const auto loaded =
        renderModels_.find(id);

    if (
        loaded !=
        renderModels_.end()
    ) {
        return loaded->second;
    }

    const eld::model::Model& model =
        getModel(id);

    std::unordered_map<
        std::uint16_t,
        const eld::texture::TextureAsset*
    > resolvedTextures;

    for (
        const eld::model::Face& face :
        model.asset.faces
    ) {
        if (!face.textureId.has_value()) {
            continue;
        }

        const std::uint16_t textureId =
            *face.textureId;

        if (
            resolvedTextures.contains(
                textureId
            )
        ) {
            continue;
        }

        try {
            const eld::texture::Texture&
                texture =
                    getTexture(
                        textureId
                    );

            resolvedTextures.emplace(
                textureId,
                &texture.asset
            );
        }
        catch (const std::exception&) {
            resolvedTextures.emplace(
                textureId,
                nullptr
            );
        }
    }

    eld::render::RenderModel renderModel =
        modelRenderBuilder_.build(
            model.asset,
            resolvedTextures
        );

    auto inserted =
        renderModels_.emplace(
            id,
            std::move(renderModel)
        );

    return inserted.first->second;
}

void AssetService::clear() {
    renderModels_.clear();
    loadedTextures_.clear();
    loadedModels_.clear();
}

}
