#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "npc/NpcData.h"
#include "npc/NpcDecoder.h"

namespace eld::npc {

class NpcLoader {
public:
  explicit NpcLoader(const eld::cache::Cache &cache);

  const NpcData &data(std::uint16_t id) const;

  std::optional<NpcData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  NpcData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "npc.dat";

  static constexpr std::string_view IndexFile = "npc.idx";

  eld::archive::Archive archive_;
  NpcDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, NpcData> dataCache_;
};

} // namespace eld::npc
