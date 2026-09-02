#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Model.h"
#include "ModelDecoder.h"
#include "cache/Store.h"

namespace eld::model {


class ModelRepository {
public:
    explicit ModelRepository(
        eld::cache::Store store
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
    eld::cache::Store store_;
    ModelDecoder decoder_;
};

}
