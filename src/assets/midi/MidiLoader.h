#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "cache/Cache.h"
#include "cache/Store.h"
#include "midi/MidiData.h"
#include "midi/MidiDecoder.h"

namespace eld::midi {

class MidiLoader {
public:
  explicit MidiLoader(const eld::cache::Cache &cache);

  const MidiData &data(std::uint16_t id) const;
  std::optional<MidiData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  MidiData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Midi;

  eld::cache::Store store_;
  MidiDecoder decoder_;
  mutable std::unordered_map<std::uint16_t, MidiData> dataCache_;
};

} // namespace eld::midi
