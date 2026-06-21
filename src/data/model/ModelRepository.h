#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "Model.h"
#include "ModelAssetBuilder.h"
#include "ModelFileParser.h"
#include "cache/Store.h"

namespace eld::model {

using ModelPredicate =
    std::function<bool(const Model&)>;

class ModelRepository {
public:
    explicit ModelRepository(
        eld::cache::Store store
    );

    std::uint16_t create(
        const ModelAsset& asset
    );

    void create(
        std::uint16_t id,
        const ModelAsset& asset
    );

    Model get(
        std::uint16_t id
    ) const;

    std::optional<Model> find(
        std::uint16_t id
    ) const;

    ModelFile getFile(
        std::uint16_t id
    ) const;

    ModelAsset getAsset(
        std::uint16_t id
    ) const;

    void update(
        std::uint16_t id,
        const ModelAsset& asset
    );

    void remove(
        std::uint16_t id
    );

    std::vector<std::uint16_t> listIds() const;

    std::vector<std::uint16_t> filterIds(
        const ModelPredicate& predicate
    ) const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

    std::size_t count(
        const ModelPredicate& predicate
    ) const;

private:
    eld::cache::Store store_;

    ModelFileParser parser_;
    ModelAssetBuilder builder_;
};

}
