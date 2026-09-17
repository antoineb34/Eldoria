#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "parameter/ParameterData.h"
#include "parameter/ParameterDecoder.h"

namespace eld::parameter {

class ParameterLoader {
public:
  explicit ParameterLoader(const eld::cache::Cache &cache);

  const ParameterData &data(std::uint16_t id) const;

  std::optional<ParameterData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  ParameterData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "param.dat";

  static constexpr std::string_view IndexFile = "param.idx";

  eld::archive::Archive archive_;
  ParameterDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, ParameterData> dataCache_;
};

} // namespace eld::parameter
