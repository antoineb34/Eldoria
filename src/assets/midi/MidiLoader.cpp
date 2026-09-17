#include "midi/MidiLoader.h"

#include <exception>
#include <stdexcept>
#include <string>

#include "cache/File.h"

namespace eld::midi {

MidiLoader::MidiLoader(const eld::cache::Cache &cache)
    : store_(cache.open(Index)) {}

MidiData MidiLoader::loadData(std::uint16_t id) const {
  const eld::cache::File cacheFile = store_.get(id);

  try {
    MidiData midi = decoder_.decode(cacheFile.getBytes());

    midi.id = id;

    return midi;
  } catch (const std::exception &error) {
    throw std::runtime_error("Failed to decode MIDI " + std::to_string(id) +
                             ": " + error.what());
  }
}

std::optional<MidiData> MidiLoader::find(std::uint16_t id) const {
  if (!contains(id)) {
    return std::nullopt;
  }

  return data(id);
}

std::vector<std::uint16_t> MidiLoader::listIds() const {
  const std::vector<eld::cache::FileEntry> entries = store_.list();

  std::vector<std::uint16_t> ids;

  ids.reserve(entries.size());

  for (const eld::cache::FileEntry &entry : entries) {
    ids.push_back(entry.fileId);
  }

  return ids;
}

bool MidiLoader::contains(std::uint16_t id) const {
  return store_.contains(id);
}

std::size_t MidiLoader::count() const { return store_.count(); }

const MidiData &MidiLoader::data(std::uint16_t id) const {
  const auto cached = dataCache_.find(id);

  if (cached != dataCache_.end()) {
    return cached->second;
  }

  const auto entry = dataCache_.emplace(id, loadData(id));

  return entry.first->second;
}

} // namespace eld::midi
