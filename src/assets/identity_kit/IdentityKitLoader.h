#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "archive/Archive.h"
#include "cache/Cache.h"
#include "identity_kit/IdentityKitData.h"
#include "identity_kit/IdentityKitDecoder.h"

namespace eld::identity_kit {

class IdentityKitLoader {
public:
  explicit IdentityKitLoader(const eld::cache::Cache &cache);

  const IdentityKitData &data(std::uint16_t id) const;

  std::optional<IdentityKitData> find(std::uint16_t id) const;

  std::vector<std::uint16_t> listIds() const;

  bool contains(std::uint16_t id) const;

  std::size_t count() const;

private:
  IdentityKitData loadData(std::uint16_t id) const;

  static constexpr auto Index = eld::cache::IndexId::Config;

  static constexpr std::uint16_t ArchiveId = 2;

  static constexpr std::string_view DataFile = "idk.dat";

  static constexpr std::string_view IndexFile = "idk.idx";

  eld::archive::Archive archive_;
  IdentityKitDecoder decoder_;

  mutable std::unordered_map<std::uint16_t, IdentityKitData> dataCache_;
};

} // namespace eld::identity_kit
