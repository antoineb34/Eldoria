#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Model.h"
#include "cache/Cache.h"
#include "decoders/ModelDecoder.h"

namespace eld::model {

class ModelRepository {
public:
    explicit ModelRepository(
        const eld::cache::Cache& cache
    );

    Model get(
        std::uint16_t id
    ) const;

    std::optional<Model> find(
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

    eld::cache::Store store_;
    ModelDecoder decoder_;
};

}
