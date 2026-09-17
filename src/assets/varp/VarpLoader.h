#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "varp/VarpData.h"
#include "varp/VarpDecoder.h"

namespace eld::varp {

class VarpLoader {
public:
  explicit VarpLoader(const eld::cache::Cache &cache);

  const VarpData &data(std::uint16_t id) const;

  std::optional<VarpData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  VarpData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "varp.dat";

  static constexpr std::string_view IndexFile = "varp.idx";

  eld::archive::Archive archive_;
  VarpDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, VarpData> dataCache_;
};

} // namespace eld::varp
