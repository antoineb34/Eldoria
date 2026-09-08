#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "Parameter.h"
#include "archive/Archive.h"
#include "cache/Cache.h"
#include "decoders/ParameterDecoder.h"

namespace eld::parameter {

class ParameterRepository {
public:
    explicit ParameterRepository(
        const eld::cache::Cache& cache
    );

    Parameter get(
        std::uint16_t id
    ) const;

    std::optional<Parameter> find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Config;

    static constexpr std::uint16_t ArchiveId =
        2;

    static constexpr std::string_view DataFile =
        "param.dat";

    static constexpr std::string_view IndexFile =
        "param.idx";

    eld::archive::Archive archive_;
    ParameterDecoder decoder_;
};

}
