#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "MidiDecoder.h"
#include "MidiFile.h"
#include "cache/Store.h"

namespace eld::midi {

class MidiRepository {
public:
    explicit MidiRepository(
        eld::cache::Store store
    );

    MidiFile get(
        std::uint16_t id
    ) const;

    std::optional<MidiFile> find(
        std::uint16_t id
    ) const;

    std::vector<std::uint16_t> listIds() const;

    bool contains(
        std::uint16_t id
    ) const;

    std::size_t count() const;

private:
    eld::cache::Store store_;
    MidiDecoder decoder_;
};

}
