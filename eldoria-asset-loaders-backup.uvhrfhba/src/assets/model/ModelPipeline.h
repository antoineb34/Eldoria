#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <vector>

#include "model/ModelData.h"
#include "model/ModelResource.h"
#include "model/ModelAssembler.h"
#include "cache/Cache.h"
#include "model/ModelDecoder.h"
#include "texture/TexturePipeline.h"

namespace eld::model {

class ModelPipeline {
public:
    ModelPipeline(
        const eld::cache::Cache& cache,
        const eld::texture::TexturePipeline& textures
    );

    const ModelData& data(
        std::uint16_t id
    ) const;

    std::optional<ModelData> find(
        std::uint16_t id
    ) const;

    const ModelResource& resource(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Models;

    ModelData loadData(
        std::uint16_t id
    ) const;

    ModelResource assembleResource(
        std::uint16_t id
    ) const;

    eld::cache::Store store_;

    const eld::texture::TexturePipeline&
        textures_;

    ModelDecoder decoder_;
    ModelAssembler assembler_;

    mutable std::map<
        std::uint16_t,
        ModelData
    > dataCache_;

    mutable std::map<
        std::uint16_t,
        ModelResource
    > resourceCache_;
};

}
