#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "Midi.h"
#include "cache/Cache.h"
#include "cache/Store.h"
#include "decoders/MidiDecoder.h"

namespace eld::midi {

class MidiRepository {
public:
    explicit MidiRepository(
        const eld::cache::Cache& cache
    );

    Midi get(
        std::uint16_t id
    ) const;

    std::optional<Midi> find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t>
    listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    static constexpr auto Index =
        eld::cache::IndexId::Midi;

    eld::cache::Store store_;
    MidiDecoder decoder_;
};

}
