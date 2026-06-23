#pragma once

#include <cstdint>
#include <unordered_map>

#include "model/ModelRepository.h"
#include "texture/TextureRepository.h"

#include "ModelRenderBuilder.h"
#include "RenderModel.h"

namespace eld::resource {

class AssetService {
public:
    AssetService(
        eld::model::ModelRepository& models,
        eld::texture::TextureRepository& textures
    );

    const eld::model::Model& getModel(
        std::uint16_t id
    );

    const eld::texture::Texture& getTexture(
        std::uint16_t id
    );

    const eld::render::RenderModel& getRenderModel(
        std::uint16_t id
    );

    void clear();

private:
    eld::model::ModelRepository& models_;
    eld::texture::TextureRepository& textures_;

    eld::render::ModelRenderBuilder
        modelRenderBuilder_;

    std::unordered_map<
        std::uint16_t,
        eld::model::Model
    > loadedModels_;

    std::unordered_map<
        std::uint16_t,
        eld::texture::Texture
    > loadedTextures_;

    std::unordered_map<
        std::uint16_t,
        eld::render::RenderModel
    > renderModels_;
};

}
