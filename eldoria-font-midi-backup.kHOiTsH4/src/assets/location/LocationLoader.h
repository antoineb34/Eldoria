#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "location/LocationData.h"
#include "location/LocationDecoder.h"

namespace eld::location {

class LocationLoader {
public:
  explicit LocationLoader(const eld::cache::Cache &cache);

  const LocationData &data(std::uint16_t id) const;

  std::optional<LocationData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  LocationData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "loc.dat";

  static constexpr std::string_view IndexFile = "loc.idx";

  eld::archive::Archive archive_;
  LocationDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, LocationData> dataCache_;
};

} // namespace eld::location
