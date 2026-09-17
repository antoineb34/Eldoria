#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "cache/Cache.h"
#include "model/ModelData.h"
#include "model/ModelDecoder.h"

namespace eld::model {

class ModelLoader {
public:
    explicit ModelLoader(
        const eld::cache::Cache& cache
    );

    const ModelData& get(
        std::uint16_t id
    ) const;

    const ModelData* find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto CacheIndex =
        eld::cache::IndexId::Models;

    ModelData load(
        std::uint16_t id
    ) const;

    eld::cache::Store store_;
    ModelDecoder decoder_;

    mutable std::unordered_map<
        std::uint16_t,
        ModelData
    > modelCache_;
};

}
