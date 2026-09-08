#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "varbit/VarbitData.h"
#include "varbit/VarbitDecoder.h"

namespace eld::varbit {

class VarbitLoader {
public:
  explicit VarbitLoader(const eld::cache::Cache &cache);

  const VarbitData &data(std::uint16_t id) const;

  std::optional<VarbitData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  VarbitData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "varbit.dat";

  static constexpr std::string_view IndexFile = "varbit.idx";

  eld::archive::Archive archive_;
  VarbitDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, VarbitData> dataCache_;
};

} // namespace eld::varbit
