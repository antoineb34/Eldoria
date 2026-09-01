#include "MidiRepository.h"

#include <stdexcept>
#include <string>
#include <utility>

#include "cache/File.h"

namespace eld::midi {

MidiRepository::MidiRepository(
    eld::cache::Store store
)
    : store_(std::move(store)) {
}

MidiFile MidiRepository::get(
    std::uint16_t id
) const {
    const eld::cache::File cacheFile =
        store_.get(id);

    std::optional<MidiFileData> data =
        parser_.parse(
            cacheFile.getBytes()
        );

    if (!data.has_value()) {
        throw std::runtime_error(
            "Failed to parse MIDI " +
            std::to_string(id)
        );
    }

    return MidiFile{
        .id = id,
        .data = std::move(*data)
    };
}

std::optional<MidiFile> MidiRepository::find(
    std::uint16_t id
) const {
    if (!contains(id)) {
        return std::nullopt;
    }

    return get(id);
}

std::vector<std::uint16_t>
MidiRepository::listIds() const {
    const std::vector<eld::cache::FileEntry> entries =
        store_.list();

    std::vector<std::uint16_t> ids;
    ids.reserve(entries.size());

    for (const eld::cache::FileEntry& entry : entries) {
        ids.push_back(entry.fileId);
    }

    return ids;
}

bool MidiRepository::contains(
    std::uint16_t id
) const {
    return store_.contains(id);
}

std::size_t MidiRepository::count() const {
    return store_.count();
}

}
