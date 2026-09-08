#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "floor/FloorData.h"
#include "floor/FloorDecoder.h"

namespace eld::floor {

class FloorLoader {
public:
  explicit FloorLoader(const eld::cache::Cache &cache);

  const FloorData &data(std::uint16_t id) const;

  std::optional<FloorData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  FloorData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "flo.dat";

  static constexpr std::string_view IndexFile = "flo.idx";

  eld::archive::Archive archive_;
  FloorDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, FloorData> dataCache_;
};

} // namespace eld::floor
