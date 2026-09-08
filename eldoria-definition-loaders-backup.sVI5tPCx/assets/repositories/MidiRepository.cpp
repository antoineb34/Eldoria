#include "repositories/MidiRepository.h"

#include <exception>
#include <stdexcept>
#include <string>

#include "cache/File.h"

namespace eld::midi {

MidiRepository::MidiRepository(
    const eld::cache::Cache& cache
)
    : store_(
          cache.open(Index)
      ) {
}


Midi MidiRepository::get(
    std::uint16_t id
) const {
    const eld::cache::File cacheFile =
        store_.get(
            id
        );

    try {
        Midi midi =
            decoder_.decode(
                cacheFile.getBytes()
            );

        midi.id = id;

        return midi;
    }
    catch (const std::exception& error) {
        throw std::runtime_error(
            "Failed to decode MIDI " +
            std::to_string(id) +
            ": " +
            error.what()
        );
    }
}


std::optional<Midi>
MidiRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(id);
}


std::vector<std::uint16_t>
MidiRepository::listIds() const {
    const std::vector<
        eld::cache::FileEntry
    > entries =
        store_.list();

    std::vector<std::uint16_t> ids;

    ids.reserve(
        entries.size()
    );

    for (
        const eld::cache::FileEntry& entry :
        entries
    ) {
        ids.push_back(
            entry.fileId
        );
    }

    return ids;
}


bool MidiRepository::contains(
    std::uint16_t id
) const {
    return store_.contains(
        id
    );
}


std::size_t MidiRepository::count() const {
    return store_.count();
}

}
